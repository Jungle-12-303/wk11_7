/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */

#include "vm/vm.h"
#include "devices/disk.h"

/* DO NOT MODIFY BELOW LINE */
static struct disk *swap_disk;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations anon_ops = {
	.swap_in = anon_swap_in,
	.swap_out = anon_swap_out,
	.destroy = anon_destroy,
	.type = VM_ANON,
};

/* Initialize the data for anonymous pages */
void
vm_anon_init (void) {
	/* TODO: Set up the swap_disk. */
	/* @todo(vm-min): 최소 lazy/stack cycle만 볼 때는 swap_disk 없이 시작할 수
	 * 있다. swap tests를 위해서는 disk_get(1, 1) 등으로 swap disk를 잡고
	 * slot bitmap과 lock을 초기화한다. */
	swap_disk = NULL;
}

/* Initialize the file mapping */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &anon_ops;

	struct anon_page *anon_page = &page->anon;
	/* @todo(vm-min): anonymous page metadata를 초기화한다. stack page는
	 * zero-filled frame으로 시작하면 충분하고, swap 구현 시 slot 없음 상태를
	 * 표시한다. */
}

/* Swap in the page by read contents from the swap disk. */
static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page = &page->anon;
	/* @todo(vm-min): swap slot이 없으면 kva 한 page를 0으로 채운다. slot이
	 * 있으면 swap disk에서 PGSIZE bytes를 읽고 slot을 free로 돌린다. */
}

/* Swap out the page by writing contents to the swap disk. */
static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page = &page->anon;
	/* @todo(vm-min): eviction 전까지는 false/PANIC으로 둘 수 있다. swap 구현
	 * 시 빈 slot을 할당하고 page->frame->kva의 4KB를 swap disk에 기록한 뒤
	 * PTE present mapping을 제거한다. */
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
static void
anon_destroy (struct page *page) {
	struct anon_page *anon_page = &page->anon;
	/* @todo(vm-min): resident frame이 있으면 frame 해제는 공통 destroy 경로와
	 * 중복되지 않게 책임을 정한다. swap slot이 남아 있으면 slot bitmap을
	 * free로 되돌린다. */
}
