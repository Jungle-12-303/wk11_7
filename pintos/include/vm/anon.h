#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;

struct anon_page {
	/* TODO VM-14: anonymous page metadata를 둔다. 최소 lazy/stack 구현만 보면
	 * 비워둘 수 있지만, swap까지 가면 swap slot index와 swapped 여부가 필요하다. */
};

void vm_anon_init (void);
bool anon_initializer (struct page *page, enum vm_type type, void *kva);

#endif
