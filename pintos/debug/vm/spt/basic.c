#include "debug/test.h"
#include "vm/vm.h"

/*
 * supplemental page table 초기화 뒤 아직 등록하지 않은 주소를 조회하면
 * 페이지를 찾지 못해야 한다는 기본 동작을 확인한다.
 */
DEBUG_TEST (vm_spt_basic) {
	struct supplemental_page_table spt;

	supplemental_page_table_init (&spt);

	ASSERT (spt_find_page (&spt, (void *) 0x1234) == NULL);
	pass ();
}
