#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

struct page;
enum vm_type;

struct file_page {
	/* @todo(vm-min): mmap/file-backed page의 lazy fault를 위해 file,
	 * file offset, read_bytes, zero_bytes, writable/dirty write-back 기준을
	 * 저장한다. do_mmap()이 page별 aux 또는 file_page metadata로 이 값을
	 * 넘기고, file_backed_swap_in/out()이 같은 값을 사용해야 한다. */
};

void vm_file_init (void);
bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);
void *do_mmap (void *addr, size_t length, int writable,
               struct file *file, off_t offset);
void do_munmap (void *va);
#endif
