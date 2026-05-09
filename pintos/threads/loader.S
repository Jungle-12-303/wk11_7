/* This file is derived from source code used in MIT's 6.828
   course.  The original copyright notice is reproduced in full
   below. */

/*
 * Copyright (C) 1997 Massachusetts Institute of Technology 
 *
 * This software is being provided by the copyright holders under the
 * following license. By obtaining, using and/or copying this software,
 * you agree that you have read, understood, and will comply with the
 * following terms and conditions:
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose and without fee or royalty is
 * hereby granted, provided that the full text of this NOTICE appears on
 * ALL copies of the software and documentation or portions thereof,
 * including modifications, that you make.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS," AND COPYRIGHT HOLDERS MAKE NO
 * REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED. BY WAY OF EXAMPLE,
 * BUT NOT LIMITATION, COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR
 * WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR
 * THAT THE USE OF THE SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY
 * THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS. COPYRIGHT
 * HOLDERS WILL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE OR
 * DOCUMENTATION.
 *
 * The name and trademarks of copyright holders may NOT be used in
 * advertising or publicity pertaining to the software without specific,
 * written prior permission. Title to copyright in this software and any
 * associated documentation will at all times remain with copyright
 * holders. See the file AUTHORS which should have accompanied this software
 * for a list of all copyright holders.
 *
 * This file may be derived from previously copyrighted software. This
 * copyright applies only to those changes made by the copyright
 * holders listed in the AUTHORS file. The rest of this file is covered by
 * the copyright notices, if any, listed below.
 */

#include "threads/loader.h"
	
#### 커널 로더.

#### 이 코드는 하드 디스크의 첫 번째 섹터에 저장되어 있어야 한다.
#### PintOS 테스트 환경에서 이 "하드 디스크"는 보통 QEMU가 제공하는 가상
#### 디스크 이미지다. QEMU는 그 이미지를 IDE 디스크처럼 보이게 만들지만,
#### 바이트 배치는 일반 부트 디스크와 같다. sector 0에는 이 512바이트 부트
#### 로더가 들어 있고, 그 뒤 섹터들에는 이 로더가 RAM으로 복사할 커널 이미지가
#### 들어 있다.
####
#### BIOS가 실행되면 이 코드를 물리 주소 0x7c00-0x7e00 범위, 즉 512바이트
#### 영역에 올린 뒤 그 시작 주소로 점프한다. 이때 CPU는 real mode다.
#### 이 코드는 전체 메모리에 접근할 수 있도록 protected mode, 즉 32비트
#### 모드로 전환하고, 커널을 메모리에 올린 뒤 start.S가 링크된 커널의 첫
#### 바이트로 점프한다.
####
#### 이 시점에는 아직 PintOS 스레드가 존재하지 않는다. CPU가 부트 로더
#### 명령어를 직접 실행하고 있을 뿐이다. PintOS의 첫 스레드는 나중에
#### init.c:main() -> thread_init() 안에서 만들어진다.
	
/* Flags in control register 0. */
#define CR0_PE 0x00000001      /* Protection Enable. */
#define CR0_EM 0x00000004      /* (Floating-point) Emulation. */
#define CR0_PG 0x80000000      /* Paging. */
#define CR0_WP 0x00010000      /* Write-Protect enable in kernel mode. */


.globl start
start:
	
# Code runs in real mode, which is a 16-bit segment.
	.code16

# Disable interrupts, because we will not be prepared to handle them
# in protected mode until much later.
# String instructions go upward (e.g. for "rep stosl" below).

	cli
	cld

# Set up data segments.

	subw %ax, %ax
	movw %ax, %es
	movw %ax, %ds

# Set up stack segment.
# Stack grows downward starting from us.
# We don't ever use the stack, but we call into the BIOS,
# which might.

	movw %ax, %ss
	movw $0x7c00, %sp
	
#### Enable A20.  Address line 20 is tied to low when the machine
#### boots, which prevents addressing memory about 1 MB.  This code
#### fixes it.
	
# Poll status register while busy.

1:	inb $0x64, %al
	testb $0x2, %al
	jnz 1b

# Send command for writing output port.

	movb $0xd1, %al
	outb %al, $0x64

# Poll status register while busy.

1:	inb $0x64, %al
	testb $0x2, %al
	jnz 1b

# Enable A20 line.

	movb $0xdf, %al
	outb %al, $0x60


#### Get memory size, via interrupt 15h function e820h. Now our pintos
#### runs on x86_64, "Get Extended Memory Size" operation is not sufficient
#### for our purpose.
	mov $0xe820, %eax  # command
	mov $(E820_MAP4), %edi  # dst
	xor %ebx, %ebx
	mov $0x534d4150, %edx # magic
	mov $24, %ecx
	int $0x15
	cmp %eax, %edx
	test %ebx, %ebx
	je panic
	mov $24, %ebp

parse_e820:
	mov %ecx, -4(%edi)
	add $24, %edi
	mov $0xe820, %eax
	mov $24, %ecx
	int $0x15
	jc e820_parse_done
	add $24, %ebp
	test %ebx, %ebx
	jne parse_e820

e820_parse_done:
	mov %ecx, -4(%edi)
	movl $0x40, MULTIBOOT_FLAG
	movl %ebp, MULTIBOOT_MMAP_LEN
	movl $(E820_MAP), MULTIBOOT_MMAP_ADDR # e820

#### Switch to protected mode.

# Note that interrupts are still off.

# Point the GDTR to our GDT.  Protected mode requires a GDT.
# We need a data32 prefix to ensure that all 32 bits of the GDT
# descriptor are loaded (default is to load only 24 bits).

	data32 lgdt gdtdesc

# Then we turn on the following bits in CR0:
#    PE (Protect Enable): this turns on protected mode.
#    PG (Paging): turns on paging.
#    WP (Write Protect): if unset, ring 0 code ignores
#       write-protect bits in page tables (!).
#    EM (Emulation): forces floating-point instructions to trap.
#       We don't support floating point. 
	
	movl %cr0, %eax
	orl $CR0_PE, %eax
	movl %eax, %cr0
	
# We're now in protected mode in a 16-bit segment.  The CPU still has
# the real-mode code segment cached in %cs's segment descriptor.  We
# need to reload %cs, and the easiest way is to use a far jump.
# Because we're not in a 32-bit segment the data32 prefix is needed to
# jump to a 32-bit offset.

	data32 ljmp $SEL_KCSEG, $protcseg

# We're now in protected mode in a 32-bit segment.

	.code32

# Reload all the other segment registers and the stack pointer to
# point into our new GDT.

protcseg:
	movw $SEL_KDSEG, %ax
	movw %ax, %ds		
	movw %ax, %es		
	movw %ax, %fs		
	movw %ax, %gs		
	movw %ax, %ss
	movl $LOADER_PHYS_BASE + 0x30000, %esp

#### IDE 컨트롤러를 직접 조작해서 커널을 물리 주소 LOADER_PHYS_BASE부터
#### 시작하는 메모리 영역에 적재한다.
####
#### %ebx = 다음에 읽을 디스크 섹터 번호.
####        sector 0은 이 로더 자신이므로 커널은 sector 1에서 시작한다.
#### %edi = RAM에 복사할 목적지 주소.
####        아래의 "rep insw"는 에뮬레이션된 IDE 데이터 포트에서 512바이트
####        디스크 섹터 하나를 메모리로 복사하고, 그만큼 %edi를 자동으로
####        증가시킨다. 루프가 끝나면 LOADER_PHYS_BASE 위치의 RAM에는
####        커널 이미지의 시작 부분이 들어 있다.

	movl $1, %ebx
	movl $LOADER_PHYS_BASE, %edi

# Disable interrupt delivery by IDE controller, because we will be
# polling for data.
# (If we don't do this, Bochs 2.2.6 will never deliver any IDE
# interrupt to us later after we reset the interrupt controller during
# boot, even if we also reset the IDE controller.)

	movw $0x3f6, %dx
	movb $0x02, %al
	outb %al, %dx
	
read_sector:

# Poll status register while controller busy.

	movl $0x1f7, %edx
1:	inb %dx, %al
	testb $0x80, %al
	jnz 1b

# Read a single sector.

	movl $0x1f2, %edx
	movb $1, %al
	outb %al, %dx

# Sector number to write in low 28 bits.
# LBA mode, device 0 in top 4 bits.

	movl %ebx, %eax
	andl $0x0fffffff, %eax
	orl $0xe0000000, %eax

# Dump %eax to ports 0x1f3...0x1f6.

	movl $4, %ecx
1:	incw %dx
	outb %al, %dx
	shrl $8, %eax
	loop 1b

# READ command to command register.

	incw %dx
	movb $0x20, %al
	outb %al, %dx

# Poll status register while controller busy.

1:	inb %dx, %al
	testb $0x80, %al
	jnz 1b

# Poll status register until data ready.

1:	inb %dx, %al
	testb $0x08, %al
	jz 1b

# Transfer sector.

	movl $256, %ecx
	movl $0x1f0, %edx
	rep insw

# Next sector.

	incl %ebx
	cmpl $KERNEL_LOAD_PAGES*8 + 1, %ebx
	jnz read_sector

#### 커널 진입점으로 점프한다.
#### 이것은 스레드 전환이 아니다. 부트 로더에서 RAM에 올라간 커널 이미지의
#### 첫 바이트로 제어를 넘기는 단순한 점프/호출이다. 링커 스크립트는 이 주소에
#### start.S의 .entry 코드를 배치해 둔다.
	movl $LOADER_PHYS_BASE, %eax
	call *%eax
	jmp panic

#### GDT

gdt:
	.quad 0x0000000000000000	# null seg
	.quad 0x00cf9a000000ffff	# code seg
	.quad 0x00cf92000000ffff  # data seg
	
gdtdesc:
	.word	0x17			# sizeof (gdt) - 1
	.long	gdt

#### Fatal error.
#### Print panic_message (with help from the BIOS) and spin.

panic:  .code16			# We only panic in real mode.
	movw $panic_message, %si
	movb $0xe, %ah
	subb %bh, %bh
1:	lodsb
	test %al, %al
2:	jz 2b			# Spin.
	int $0x10
	jmp 1b

panic_message:
	.ascii "Panic!"
	.byte 0

#### Command-line arguments and their count.
#### This is written by the `pintos' utility and read by the kernel.
#### The loader itself does not do anything with the command line.
	.org LOADER_ARG_CNT - LOADER_BASE
arg_cnt:
	.long 0
	.org LOADER_ARGS - LOADER_BASE
args:
	.fill 0x80, 1, 0

#### Boot-sector signature.
#### The BIOS checks that this is set properly.
	.org LOADER_SIG - LOADER_BASE
	.word 0xaa55
