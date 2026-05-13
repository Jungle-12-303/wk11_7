/*
 * 파일 이름과 DEBUG_TEST() 내 변수명은 같아야합니다.
 */

#include "debug/test.h"

DEBUG_TEST (debug_test) {
	int value = 21 * 2;

	msg ("debug_test value=%d", value);
	ASSERT (value == 42);
	pass ();
}
