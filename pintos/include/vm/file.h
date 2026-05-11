#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

struct page;
enum vm_type;

struct file_page {
	/* TODO VM-15: mmap/file-backed page의 lazy fault 정보를 저장한다.
	 * file, file offset, read_bytes, zero_bytes, writable 여부, mmap region
	 * 식별 정보를 두고 file_backed_swap_in/out()과 do_mmap/munmap이 공유한다. */
};

void vm_file_init (void);
bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);
void *do_mmap (void *addr, size_t length, int writable,
               struct file *file, off_t offset);
void do_munmap (void *va);
#endif
