#include "debug/test.h"

/*
 * 임시 계산값을 검증하는 예제 테스트다.
 * 새 디버그 테스트를 만들 때 함수 이름과 파일 경로 규칙을 확인하는 용도로 쓴다.
 */
DEBUG_TEST (example_temp) {
	int a = 10;

	ASSERT (a == 10);
	pass ();
}
