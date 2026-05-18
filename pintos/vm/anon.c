/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */

#include "vm/vm.h"
#include "devices/disk.h"
#include "bitmap.h"
#include "threads/mmu.h"

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

#define SWAP_SLOT 8

static struct bitmap* disk_bitmap;

/* Initialize the data for anonymous pages */
void
vm_anon_init (void) {
	/* TODO: Set up the swap_disk. */
	swap_disk = disk_get(1,1);
	if (!swap_disk)
		PANIC("NO DISK");

	disk_sector_t sectors = disk_size(swap_disk);

	disk_bitmap = bitmap_create(sectors / SWAP_SLOT);
	if (!disk_bitmap)
		PANIC("NO DISK BITMAP");
}

/* Initialize the file mapping */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	ASSERT(page != NULL);
	
	page->operations = &anon_ops;

	struct anon_page *anon_page = &page->anon;

	return true;
}

/* Swap in the page by read contents from the swap disk. */
static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page = &page->anon;
	bitmap_flip(disk_bitmap, anon_page->swap_idx);

	for (int i = 0; i < SWAP_SLOT; i++){
		disk_read(swap_disk, anon_page->swap_idx * 8 + i, (int)page->frame->kva + i * 512);
	}

	pml4_set_page(thread_current()->pml4, page->va, kva, page->writable);
	return true;
}

/* Swap out the page by writing contents to the swap disk. */
static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page = &page->anon;
	anon_page->swap_idx = bitmap_scan_and_flip(disk_bitmap, 0, 1, false);
	if (anon_page->swap_idx == BITMAP_ERROR)
		return false;
	for (int i = 0; i < SWAP_SLOT; i++){
		disk_write(swap_disk, anon_page->swap_idx * 8 + i, (int)page->frame->kva + i * 512);
	}
	pml4_clear_page(page->frame->owner_thread, page->va);
	return true;
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
static void
anon_destroy (struct page *page) {
	struct anon_page *anon_page = &page->anon;
}
