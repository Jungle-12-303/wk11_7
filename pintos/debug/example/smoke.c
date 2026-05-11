/*
 * debug-test 도구가 pintos/debug 아래의 테스트 파일을 찾아서
 * 커널 테스트 목록에 등록하고 실행하는지 확인하는 가장 작은 예제다.
 *
 * 호스트 macOS/Linux에서 바로 실행해 텍스트 출력 예제로도 확인할 수 있다.
 *
 *   gcc pintos/debug/example/smoke.c -o /tmp/pintos-debug-smoke
 *   /tmp/pintos-debug-smoke
 */

#if defined(__has_include)
#if __has_include("debug/test.h")
#include "debug/test.h"
#define PINTOS_DEBUG_TEST_BUILD 1
#endif
#endif

#ifdef PINTOS_DEBUG_TEST_BUILD

DEBUG_TEST (example_smoke) {
	volatile int one = 1;

	msg ("hello, PintOS debug text");
	ASSERT (one == 1);
	pass ();
}

#else

#include <stdio.h>

int
main (void) {
	puts ("hello, PintOS debug text");
	return 0;
}

#endif
