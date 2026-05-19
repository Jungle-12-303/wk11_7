/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "devices/disk.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);
static struct lock file_lock;

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
	lock_init(&file_lock);
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;

	
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page = &page->file;
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable,
		struct file *file, off_t offset) {
	void *start_addr = addr;
	struct file *reopened_file = file_reopen(file);
	while (length > 0){
		struct lazy_load_file_arg* aux = malloc(sizeof(struct lazy_load_file_arg));
		RETURN_VALUE_IF (aux == NULL, false);
		size_t page_read_bytes = length < PGSIZE ? length : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		aux->file = reopened_file;
		if (aux->file == NULL) {
			free (aux);
			return false;
		}
		aux->ofs = offset;
		aux->page_read_bytes = page_read_bytes;
		aux->page_zero_bytes = page_zero_bytes;
		
		if (!vm_alloc_page_with_initializer (VM_FILE, addr, writable,
		                                     lazy_load_file, aux)) {
				free(aux);
				return false;																		
			}	
		/*
		 * 다음 페이지로 진행한다.
		 */
		length -= page_read_bytes;
		addr += PGSIZE;
		offset += page_read_bytes;
	}

	
	return start_addr;
}

/* Do the munmap */
void
do_munmap (void *addr) {
}

static bool
lazy_load_file (struct page *page, void *aux_) {
	struct lazy_load_file_arg *aux = aux_;
	uint8_t *kva = page->frame->kva;
	bool success = false;

	if (aux == NULL || aux->file == NULL)
		goto done;

	lock_acquire (&file_lock);
	if (file_read_at (aux->file, kva, aux->page_read_bytes, aux->ofs) == (off_t) aux->page_read_bytes) {
		memset (kva + aux->page_read_bytes, 0, aux->page_zero_bytes);
		success = true;
	}
	lock_release (&file_lock);

	done:
	if (aux != NULL) {
		if (aux->file != NULL) {
			lock_acquire (&file_lock);
			file_close (aux->file);
			lock_release (&file_lock);
		}
		free (aux);
	}
	return success;
}


