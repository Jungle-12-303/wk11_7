/*
 * 디버그 폴더에 있던 독립 실행 예제다.
 *
 * PintOS 커널 테스트 하네스에 묶이지 않는 파일이므로 공개 테스트 폴더 밖에 둔다.
 *
 * 실행 예:
 *
 *   gcc pintos/tools/debug-test/examples/debug.c -o /tmp/pintos-debug-example
 *   /tmp/pintos-debug-example
 */

#include <stdio.h>
#include <string.h>

int
main (void) {
	const char *path = "/tmp/pintos-debug-text-write-example.txt";
	const char *message = "hello, PintOS debug text";
	char buffer[64];
	FILE *file;

	file = fopen (path, "w");
	if (file == NULL)
		return 1;

	fputs (message, file);
	fclose (file);

	file = fopen (path, "r");
	if (file == NULL)
		return 1;

	if (fgets (buffer, sizeof buffer, file) == NULL) {
		fclose (file);
		return 1;
	}
	fclose (file);

	if (strcmp (buffer, message) != 0)
		return 1;

	puts (buffer);
	return 0;
}
