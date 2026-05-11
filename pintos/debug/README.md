# PintOS Debug Tests

이 폴더는 작은 커널 조각을 직접 검증하는 로컬 테스트 파일을 두는 곳이다.
현재 예제 테스트는 `pintos/debug/example/smoke.c` 하나만 둔다.

실행 스크립트와 자동 생성 파일은 `pintos/tools/debug-test/`로 분리되어 있다.
일반적으로 직접 열거나 수정하지 않는다.

## 테스트 파일 만들기

파일 경로가 테스트 이름과 함수 이름이 된다.

```text
pintos/debug/example/smoke.c
  -> test name: debug-example-smoke
  -> aliases   : smoke, example/smoke
  -> function  : debug_test_example_smoke
```

파일 내용은 아래 형태로 작성한다.

```c
#include "debug/test.h"

DEBUG_TEST (example_smoke) {
  volatile int one = 1;

  msg ("hello, PintOS debug text");
  ASSERT (one == 1);
  pass ();
}
```

규칙:

- `DEBUG_TEST(...)` 안의 이름은 파일 경로에서 만든 suffix와 맞춘다.
- `example/smoke.c`는 `DEBUG_TEST (example_smoke)`를 쓴다.
- 한 파일에는 `DEBUG_TEST (...)`를 하나만 둔다.
- 성공한 테스트는 마지막에 `pass ();`를 호출한다.
- 실패 조건은 `ASSERT (...)` 또는 `fail ("message")`로 표현한다.

## 실행하기

repo 루트에서 실행한다.

```sh
make test-list
make test TEST=smoke
make test-debug TEST=smoke
```

출력에 아래처럼 나오면 통과다.

```text
pass debug-example-smoke
All 1 debug tests passed.
```

실패하면 로그는 아래에 남는다.

```text
pintos/vm/build/debug-test-results/<test-name>.output
pintos/vm/build/debug-test-results/<test-name>.errors
```

## VSCode에서 중단점 잡기

VSCode의 빨간 점 중단점을 쓰려면 터미널에서 `make test-debug`를 실행하지 않는다.
VSCode 디버그 어댑터가 직접 GDB에 붙어야 빨간 점 중단점이 적용된다.

순서:

1. `pintos/debug/.../*.c` 파일에 빨간 점 중단점을 찍는다.
2. VSCode 왼쪽 `Run and Debug` 패널을 연다.
3. `테스트 디버거`를 선택한다.
4. 테스트 이름을 입력한다. 예: `smoke`
5. 실행한다.

내부 흐름은 아래와 같다.

```text
make test-debug-wait TEST=<입력한 테스트 이름>
QEMU --gdb 대기
VSCode GDB attach
VSCode 중단점 등록
continue
```

## 터미널 GDB로 디버그하기

터미널에서 GDB를 자동으로 붙이고 싶으면 아래 명령을 쓴다.

```sh
make test-debug TEST=smoke
```

기본 중단점은 테스트 함수다.

```text
smoke -> break debug_test_example_smoke
```

다른 함수에서 먼저 멈추고 싶으면 `BREAK`를 지정한다.

```sh
make test-debug TEST=smoke BREAK=debug_test_example_smoke
```

`TEST=`에는 긴 이름 대신 짧은 별칭을 쓸 수 있다.

```text
smoke
smoke.c
example/smoke
example/smoke.c
debug-example-smoke
```

파일명이 여러 개와 겹치면 `ambiguous debug test`가 나오며, 그때만 `example/smoke`처럼 조금 더 긴 이름을 쓰면 된다.
