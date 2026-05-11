/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

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
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
	/* @todo(vm-min): uninit aux 또는 do_mmap() metadata에서 file, offset,
	 * read_bytes, zero_bytes, writable 정보를 file_page로 옮긴다. */
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
	/* @todo(vm-min): file_page->file의 offset에서 read_bytes만큼 kva로 읽고,
	 * 나머지 zero_bytes는 memset(0)한다. short read면 실패로 반환한다. */
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
	/* @todo(vm-min): writable mmap page가 dirty이면 file_write_at()으로
	 * read_bytes 범위만 원래 file offset에 write-back한다. clean page나
	 * read-only page는 disk write 없이 성공 처리한다. */
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
	/* @todo(vm-min): process exit/munmap 경로에서 file_backed_swap_out()을
	 * 먼저 호출할지 정하고, file reference와 mmap metadata를 해제한다. */
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable,
		struct file *file, off_t offset) {
	/* @todo(vm-min): addr NULL/page-align/user range, length > 0, fd file 유효,
	 * offset page-align, 기존 SPT/PML4/stack/code/data와 겹치지 않음을 먼저
	 * 검사한다. 실패하면 MAP_FAILED를 반환한다. */
	/* @todo(vm-min): file_reopen()으로 mapping 전용 file reference를 만들고,
	 * length를 page 단위로 쪼개 각 page를 VM_FILE lazy page로 SPT에 등록한다.
	 * page별 aux에는 file, offset, read_bytes, zero_bytes, writable을 넣는다. */
}

/* Do the munmap */
void
do_munmap (void *addr) {
	/* @todo(vm-min): addr로 시작한 mmap region의 page들을 찾아 dirty page는
	 * write-back하고 PTE/SPT/frame/file metadata를 정리한다. region 단위
	 * metadata가 없다면 시작 addr에서 연속 file-backed page를 추적할 기준이
	 * 필요하다. */
}
