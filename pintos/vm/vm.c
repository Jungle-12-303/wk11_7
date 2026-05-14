/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
	/* TODO VM-04: frame table/list와 lock을 초기화한다. eviction을 아직
	 * 구현하지 않더라도 vm_get_frame()이 만든 frame을 추적해야 cleanup과
	 * 이후 eviction 구현으로 이어진다. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
	case VM_UNINIT:
		return VM_TYPE (page->uninit.type);
	default:
		return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* 어떤 page가 몇 번 버킷으로 갈지 결정하는 해시값 만들기 */
uint64_t
page_hash (const struct hash_elem *e, void *aux UNUSED) {
	const struct page *page = hash_entry (e, struct page, hash_elem);
	return hash_bytes (&page->va, sizeof page->va);
}

/* 버킷 안에서 같은 page인지 비교하기 */
bool
page_less (const struct hash_elem *a, const struct hash_elem *b,
           void *aux UNUSED) {
	const struct page *pa = hash_entry (a, struct page, hash_elem);
	const struct page *pb = hash_entry (b, struct page, hash_elem);
	return pa->va < pb->va;
}

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
                                vm_initializer *init, void *aux) {
	ASSERT (VM_TYPE (type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		/* TODO VM-05: upage를 pg_round_down() 기준으로 맞추고 type에 따라
		 * anon_initializer 또는 file_backed_initializer를 고른다. malloc()으로
		 * struct page를 만들고 uninit_new(page, upage, init, type, aux,
		 * initializer)를 호출한 뒤 writable 같은 접근권한 metadata를 page에
		 * 저장한다. */

		/* TODO: Insert the page into the spt. */
		/* TODO VM-06: spt_insert_page() 성공 시 true를 반환한다. 실패하면
		 * aux 소유권을 정해 page/aux를 누수 없이 해제한다. */
	}
err:
	return false;
}

/* 찾고 싶은 va를 pg_round_down
    임시 page.va에 넣기
    hash_find
    찾은 hash_elem을 struct page로 변환(=hash_elem을 멤버로 포함하고 있는 struct page의 시작 주소를 구한다는 뜻) */
struct page *
spt_find_page (struct supplemental_page_table *spt, void *va) {
	struct page real_page;
	struct hash_elem *e;
	real_page.va = pg_round_down (va);
	e = hash_find (&spt->hash_page, &real_page.hash_elem); /* e와 같은 요소 찾아서 반환 */
	if (e == NULL)
		return NULL;
	return hash_entry (e, struct page, hash_elem);
}

/* SPT에 새 struct page를 등록하려는 함수
같은 va가 있으면 false
같은 va가 없으면 true */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED, struct page *page UNUSED) {
	int succ = false;
	struct hash_elem *old;
	old = hash_insert (&spt->hash_page, &page->hash_elem);
	if (old == NULL)
		succ = true;
	return succ;
}

/* page를 SPT 자료구조에서 먼저 제거한 뒤 vm_dealloc_page()를
 * 호출한다. munmap(), process exit, 실패 rollback이 같은 제거 경로를
 * 재사용하게 만든다. */
/* 이제 이 페이지 안 쓸거니까 삭제한다 */
void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	/* TODO: The policy for eviction is up to you. */
	/* TODO VM-20: 처음에는 eviction을 구현하지 않고 NULL/PANIC 경로로 둘 수
	 * 있지만, swap tests를 돌리려면 frame table에서 victim을 고르는 정책
	 * clock/second chance 등을 여기에 둔다. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */
	/* TODO VM-21: victim->page에 대해 swap_out(page)을 호출하고, old PTE를
	 * 제거한 뒤 victim->page를 NULL로 되돌려 재사용 가능한 frame으로 반환한다. */

	return NULL;
}

/* user pool에서 실제 RAM frame 하나를 확보하고, 커널이 그 RAM을 다룰 수 있도록 kva를 반환하는 함수
palloc()으로 페이지를 할당받아 frame을 얻는다.
(이 아래부터는 다음에 구현하라고 깃북에 적혀있음)
사용 가능한 페이지가 없으면, 기존 페이지 하나를 eviction 해서 그 공간을 확보한 뒤 반환한다.
이 함수는 항상 유효한 주소를 반환한다.
즉, 유저 풀 메모리가 가득 차 있으면 사용 중인 frame 하나를 내쫓아서(evict) 사용 가능한 메모리 공간을 만든다.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame;
	/* TODO: Fill this function. */
	/* TODO VM-10: palloc_get_page(PAL_USER)로 kva를 얻고 struct frame을
	 * 할당해 frame->kva=kva, frame->page=NULL로 초기화한다. palloc 실패 시
	 * vm_evict_frame()으로 재사용 frame을 얻는다
	 * palloc_get_page를 호출하여 사용자 풀에서 새로운 물리 페이지를 가져옵니다.
	 */
	void *kva;
	kva = palloc_get_page (PAL_USER);
	if (kva == NULL)
		PANIC ("todo");

	frame = malloc (sizeof *frame);
	if (frame == NULL)
		PANIC ("todo");

	frame->kva = kva;
	frame->page = NULL;
	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);

	return frame; /* 초기화된 frame을 반환 */
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
	/* TODO VM-13: addr을 page boundary로 내린 뒤 VM_ANON stack page를
	 * SPT에 등록하고 즉시 vm_claim_page()한다. USER_STACK 경계와 최대 stack
	 * 크기 제한을 함께 검사해야 한다. */
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
	/* TODO VM-22: copy-on-write를 구현하지 않는 최소 cycle에서는 false를
	 * 반환해 write-protected fault를 죽인다. COW를 할 때는 새 frame 복사와
	 * writable PTE 재설치를 여기서 처리한다. */
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
                     bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO VM-12: kernel address, NULL, not_present=false인 protection fault,
	 * write인데 page가 writable이 아닌 경우를 걸러낸다. */
	/* TODO: Your code goes here */
	/* TODO VM-12: addr을 pg_round_down()해서 SPT에서 page를 찾는다. 없으면
	 * f->rsp 근처 fault인지 검사해 stack growth 후보만 vm_stack_growth()로
	 * 만든다. 찾은 page는 vm_do_claim_page()로 frame/PTE/swap_in을 연결한다. */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */
	/* TODO VM-11: va를 page boundary로 내리고 spt_find_page()로 page를
	 * 찾는다. page가 없거나 이미 frame이 있으면 정책에 맞게 처리하고, 있으면
	 * vm_do_claim_page(page)를 호출한다. */

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	/* TODO VM-11: pml4_set_page(thread_current()->pml4, page->va,
	 * frame->kva, writable)를 호출한다. 실패하면 page/frame link와 frame
	 * allocation을 되돌려야 한다. 성공한 뒤 swap_in(page, frame->kva)로
	 * lazy loader/file/anon 초기화를 실행한다. */

	return swap_in (page, frame->kva);
}

/* SPT의 hash_table을 page_hash/page_less 규칙으로 초기화한다 */
void
supplemental_page_table_init (struct supplemental_page_table *spt) {
	hash_init (&spt->hash_page, page_hash, page_less, NULL);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
                              struct supplemental_page_table *src UNUSED) {
	/* TODO VM-18: fork 최소 통과를 위해 src의 각 page를 dst에 복제한다.
	 * UNINIT은 aux를 deep copy하거나 file reference를 다시 열고, resident
	 * page는 child page를 만들고 claim한 뒤 frame bytes를 복사한다. */
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	/* TODO VM-19: SPT의 모든 page를 순회하며 dirty mmap page는 write-back,
	 * resident frame은 palloc_free_page(), file/aux metadata는 해제한다.
	 * 순회 중 SPT entry 제거와 page destroy 순서를 분명히 해야 한다. */
}
