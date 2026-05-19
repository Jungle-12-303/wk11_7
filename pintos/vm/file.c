/* file.c: 메모리에 매핑된 파일 객체, 즉 mmap 객체 구현. */

#include "vm/vm.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* 이 구조체는 수정하지 말 것. */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* 파일 기반 VM을 초기화한다. */
void
vm_file_init (void) {
}

/* 파일 기반 페이지를 초기화한다. */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva UNUSED) {
	struct file_page *file_page;

	RETURN_VALUE_IF (page == NULL, false);
	page->operations = &file_ops;

	file_page = &page->file;
	file_page->type = type;
	file_page->file = NULL;
	file_page->ofs = 0;
	file_page->read_bytes = 0;
	file_page->zero_bytes = 0;
	file_page->map_start = NULL;

	return true;
}

/* 파일에서 내용을 읽어 페이지를 swap in한다. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* 내용을 파일에 쓰기 반영해 페이지를 swap out한다. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* 파일 기반 페이지를 파괴한다. PAGE 자체는 호출자가 해제한다. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* mmap을 수행한다. */
void *
do_mmap (void *addr, size_t length, int writable,
		struct file *file, off_t offset) {
}

/* munmap을 수행한다. */
void
do_munmap (void *addr) {
}
