#ifndef DEBUG_TEST_H
#define DEBUG_TEST_H

#include "tests/threads/tests.h"
#if defined(__has_include)
#if __has_include("tools/debug-test/generated/debug-tests.h")
#include "tools/debug-test/generated/debug-tests.h"
#endif
#endif
#include <debug.h>

/*
 * 파일 경로에서 계산한 suffix와 같은 이름을 넣으면 디버그 하네스가
 * debug_test_<name> 함수를 테스트 목록에 연결한다.
 */
#define DEBUG_TEST(name) void debug_test_##name (void)

#endif /* debug/test.h */
