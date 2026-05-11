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
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
	/* @todo(vm-min): frame table/list와 lock을 초기화한다. eviction을 아직
	 * 구현하지 않더라도 vm_get_frame()에서 만든 frame을 추적해야 process exit
	 * cleanup과 이후 eviction으로 자연스럽게 이어진다. */
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

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		/* @todo(vm-min): upage를 pg_round_down() 기준으로 맞추고, type에 따라
		 * anon_initializer 또는 file_backed_initializer를 고른다. malloc()으로
		 * struct page를 만들고 uninit_new(page, upage, init, type, aux,
		 * initializer)를 호출한 뒤 writable 같은 접근권한 metadata를 page에
		 * 저장할 위치를 마련한다. */

		/* TODO: Insert the page into the spt. */
		/* @todo(vm-min): spt_insert_page() 성공 시 true를 반환한다. 실패하면
		 * aux 소유권이 누구에게 있는지 정하고 page/aux를 누수 없이 해제한다. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function. */
	/* @todo(vm-min): va를 pg_round_down()으로 page base에 맞춘 뒤 SPT에서
	 * 정확히 같은 user VA의 struct page를 찾는다. page fault, mmap overlap
	 * 검사, vm_claim_page()가 모두 이 함수를 공통 입구로 사용한다. */

	return page;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */
	/* @todo(vm-min): page->va가 이미 SPT에 있으면 실패해야 한다. 성공하면
	 * page-rounded VA를 key로 등록한다. mmap-overlap/code/data/stack 검사의
	 * 신뢰성이 여기서 갈린다. */

	return succ;
}

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
	/* @todo(vm-min): 처음에는 eviction을 구현하지 않고 NULL/PANIC 경로로 둘 수
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
	/* @todo(vm-min): victim->page에 대해 swap_out(page)을 호출하고, old PTE를
	 * 제거한 뒤 victim->page를 NULL로 되돌려 재사용 가능한 frame으로 반환한다. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */
	/* @todo(vm-min): palloc_get_page(PAL_USER)로 kva를 얻고 struct frame을
	 * 할당해 frame->kva=kva, frame->page=NULL로 초기화한다. palloc 실패 시
	 * vm_evict_frame()으로 재사용 frame을 얻는다. */

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
	/* @todo(vm-min): addr을 page boundary로 내린 뒤 VM_ANON stack page를
	 * SPT에 등록하고 즉시 vm_claim_page()한다. USER_STACK 경계와 최대 stack
	 * 크기 제한을 함께 검사해야 한다. */
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
	/* @todo(vm-min): copy-on-write를 구현하지 않는 최소 cycle에서는 false를
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
	/* @todo(vm-min): kernel address, NULL, not_present=false인 protection fault,
	 * write인데 page가 writable이 아닌 경우를 걸러낸다. */
	/* TODO: Your code goes here */
	/* @todo(vm-min): addr을 pg_round_down()해서 SPT에서 page를 찾는다. 없으면
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
	/* @todo(vm-min): va를 page boundary로 내리고 spt_find_page()로 page를
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
	/* @todo(vm-min): pml4_set_page(thread_current()->pml4, page->va,
	 * frame->kva, writable)를 호출한다. 실패하면 page/frame link와 frame
	 * allocation을 되돌려야 한다. 성공한 뒤 swap_in(page, frame->kva)로
	 * lazy loader/file/anon 초기화를 실행한다. */

	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	/* @todo(vm-min): include/vm/vm.h의 supplemental_page_table 안에 둔
	 * hash/table/list를 초기화한다. process_exec(), process_fork()의 새
	 * thread가 반드시 빈 SPT를 갖고 시작해야 한다. */
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
	/* @todo(vm-min): fork 최소 통과를 위해 src의 각 page를 dst에 복제한다.
	 * UNINIT은 aux를 deep copy하거나 file reference를 다시 열고, resident
	 * page는 child page를 만들고 claim한 뒤 frame bytes를 복사한다. */
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	/* @todo(vm-min): SPT의 모든 page를 순회하며 dirty mmap page는 write-back,
	 * resident frame은 palloc_free_page(), file/aux metadata는 해제한다.
	 * 순회 중 SPT entry 제거와 page destroy 순서를 분명히 해야 한다. */
}
