#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;

struct anon_page {
	/* @todo(vm-min): stack/lazy executable page가 resident 상태로만 도는
	 * 최소 cycle에서는 비워둘 수 있다. swap까지 연결할 때는 swap slot index,
	 * swapped 여부 같은 metadata를 여기에 둔다. */
};

void vm_anon_init (void);
bool anon_initializer (struct page *page, enum vm_type type, void *kva);

#endif
