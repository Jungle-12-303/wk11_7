#include "threads/loader.h"

/*
 * 이 파일은 실행 중에 동작하는 로더 코드가 아니라 링커 스크립트다.
 * 빌드 시스템은 이 규칙을 보고 최종 커널 ELF 이미지 안에서 각 섹션이 어느
 * 주소에 놓일지 결정한다. 이후 loader.S가 그 이미지를 가상 디스크에서 RAM으로
 * 복사하고, start.S는 여기서 배치된 진입 심벌에서 실행을 시작한다.
 */
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)			/* 커널은 "_start" 심벌에서 시작한다. */

SECTIONS
{
  /* Specifies the virtual address for the kernel base. */
	. = LOADER_KERN_BASE + LOADER_PHYS_BASE;

	PROVIDE(start = .);
  /* Kernel starts with code, followed by read-only data and writable data. */
	.text : AT(LOADER_PHYS_BASE) {
		*(.entry)
		*(.text .text.* .stub .gnu.linkonce.t.*)
	} = 0x90
	.rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }

	. = ALIGN(0x1000);
	PROVIDE(_end_kernel_text = .);

  .data : { *(.data) *(.data.*)}

  /* BSS (zero-initialized data) is after everything else. */
  PROVIDE(_start_bss = .);
  .bss : { *(.bss) }
  PROVIDE(_end_bss = .);

  PROVIDE(_end = .);

	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack .stab)
	}
}
