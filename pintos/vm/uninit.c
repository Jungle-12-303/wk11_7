/* uninit.c: 초기화되지 않은 페이지 구현.
 *
 * 모든 페이지는 uninit page로 태어난다. 첫 페이지 폴트가 발생하면 처리 경로가
 * uninit_initialize(page->operations.swap_in)를 호출한다.
 * uninit_initialize 함수는 페이지 객체를 초기화해 page를 특정 페이지 객체
 * (anon, file, page_cache)로 바꾸고, vm_alloc_page_with_initializer()에서
 * 전달한 초기화 콜백을 호출한다.
 * */

#include "vm/vm.h"
#include "vm/uninit.h"
#include "filesys/file.h"
#include "threads/malloc.h"

static bool uninit_initialize (struct page *page, void *kva);
static void uninit_destroy (struct page *page);

/* 이 구조체는 수정하지 말 것. */
static const struct page_operations uninit_ops = {
	.swap_in = uninit_initialize,
	.swap_out = NULL,
	.destroy = uninit_destroy,
	.type = VM_UNINIT,
};

/* 이 함수는 수정하지 말 것. */
void
uninit_new (struct page *page, void *va, vm_initializer *init,
		enum vm_type type, void *aux,
		bool (*initializer)(struct page *, enum vm_type, void *)) {
	ASSERT (page != NULL);

	*page = (struct page) {
		.operations = &uninit_ops,
		.va = va,
		.frame = NULL, /* 아직 프레임이 없다. */
		.uninit = (struct uninit_page) {
			.init = init,
			.type = type,
			.aux = aux,
			.page_initializer = initializer,
		}
	};
}

/* 첫 폴트에서 페이지를 초기화한다. */
static bool
uninit_initialize (struct page *page, void *kva) {
	RETURN_VALUE_IF (page == NULL, false);
	struct uninit_page *uninit = &page->uninit;
	vm_initializer *init = uninit->init;
	enum vm_type type = uninit->type;
	void *aux = uninit->aux;
	bool (*initializer) (struct page *, enum vm_type, void *) =
		uninit->page_initializer;

	RETURN_VALUE_IF (initializer == NULL, false);
	return initializer (page, type, kva) && (init == NULL || init (page, aux));
}

/* uninit_page가 가진 자원을 해제한다. 대부분의 페이지는 다른 페이지 객체로
 * 바뀌지만, 실행 중 한 번도 참조되지 않은 uninit page가 프로세스 종료 시점에
 * 남아 있을 수 있다. PAGE 자체는 호출자가 해제한다. */
static void
uninit_destroy (struct page *page) {
	RETURN_IF (page == NULL);
	struct lazy_load_arg *aux = page->uninit.aux;

	RETURN_IF (aux == NULL);
	if (aux->file != NULL)
		file_close (aux->file);
	free (aux);
	page->uninit.aux = NULL;
}
