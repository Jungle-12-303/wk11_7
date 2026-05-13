/*
 * debug-test 하네스가 pintos/debug 아래의 테스트 파일을 자동 등록하고
 * 커널 테스트로 실행하는지 확인하는 가장 작은 팀 공유용 테스트다.
 */

#include "debug/test.h"

DEBUG_TEST (debug_test) {
	int value = 21 * 2;

	msg ("debug_test value=%d", value);
	ASSERT (value == 42);
	pass ();
}
