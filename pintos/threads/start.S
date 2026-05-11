#include "threads/loader.h"
#define LONG_MODE (1 << 29)
#define CR0_PE 0x00000001
#define CR0_PG (1 << 31)
#define CR4_PAE 0x20
#define PTE_P 0x1
#define PTE_W 0x2
#define EFER_MSR 0xC0000080
#define EFER_LME (1 << 8)
#define EFER_SCE (1 << 0)
#define RELOC(x) (x - LOADER_KERN_BASE)
.section .entry

.globl _start
_start = RELOC(bootstrap)

.globl bootstrap
.func bootstrap
.code32
#### 64비트 코드로 진입하기 위한 부트스트랩이다.
####
#### loader.S는 이미 커널 이미지를 RAM으로 복사한 뒤 물리 주소
#### LOADER_PHYS_BASE로 제어를 넘겼다. 여기의 코드는 처음 실행되는 커널
#### 코드다. 아직 PintOS 하위 시스템을 초기화하는 단계가 아니라, 64비트 C
#### 코드를 안전하게 실행할 수 있을 만큼의 CPU 상태만 만든다.
####
#### 중요한 경계:
#### - init.c:main() 전에는 PintOS 스레드 추상화가 없다.
#### - init.c:main() -> thread_init() 이후에는 현재 실행 중인 커널 흐름이
####   최초의 "main" 스레드로 표현된다.
bootstrap:
	pushf
	pop %eax
	mov %ecx, %eax
	xor $0x200000, %eax
	push %eax
	popf
	cmp %eax, %ebx
	jz no_long_mode # Check cpuid instruction exist.
	xor %eax, %eax
	cpuid           # query cpuid 1.
	cmp $1, %eax
	jb no_long_mode
	test $LONG_MODE, %edx
#### Enable Physical Address Extension
	movl %cr4, %eax
	orl $CR4_PAE, %eax
	movl %eax, %cr4


#### Create page directory and page table and
#### set page directory base register (cr3).
setup_page_table:
# 1. fill boot_pml4e with zeros
  lea (RELOC(boot_pml4e)), %edi
	xor %eax, %eax
	mov $0x400, %ecx
	rep stosl (%edi)

# 2. set pdpts
  lea (RELOC(boot_pml4e)), %edi
	lea (RELOC(boot_pdpt1)), %ebx
	orl $(PTE_P | PTE_W), %ebx
	mov %ebx, (%edi) # pdpt1
	lea (RELOC(boot_pdpt2)), %ebx
	orl $(PTE_P | PTE_W), %ebx
	mov %ebx, 8(%edi) # pdpt2

# 3. set pdpes
  lea (RELOC(boot_pdpt1)), %edi
	lea (RELOC(boot_pde1)), %ebx
	orl $(PTE_P | PTE_W), %ebx
	mov %ebx, (%edi)

  lea (RELOC(boot_pdpt2)), %edi
	lea (RELOC(boot_pde2)), %ebx
	orl $(PTE_P | PTE_W), %ebx
	mov %ebx, (%edi)

# 4. setup pdes
  mov $128, %ecx
	lea (RELOC(boot_pde1)), %ebx
	lea (RELOC(boot_pde2)), %edx
	add $256, %edx
	mov $(PTE_P | PTE_W | 0x180), %eax

fill_pdes:
	mov %eax, (%ebx)
	mov %eax, (%edx)
	add $8, %ebx
	add $8, %edx
	add $0x200000, %eax
	dec %ecx
	cmp $0, %ecx
	jne fill_pdes

# 5. Load page directory base register (cr3).
	lea (RELOC(boot_pml4e)), %eax
	mov %eax, %cr3

#### Enable the long mode using MSR (Model Specific Register)
#### Enable syscall (EFER_SCE)
	mov $EFER_MSR, %ecx
	rdmsr
	orl $(EFER_LME | EFER_SCE), %eax
	wrmsr

#### Enable paging
	mov %cr0, %eax
	or $(CR0_PE|CR0_PG), %eax
	mov %eax, %cr0

#### Jump to the long mode
	lea (RELOC(gdt_desc64)), %eax
	lgdt (%eax)
	mov $(entry_64 - LOADER_KERN_BASE), %eax
	push $SEL_KCSEG
	push %eax
	lret
.endfunc

no_long_mode:
	jmp no_long_mode

.p2align 2
gdt64:
  .quad 0                   # NULL SEGMENT
  .quad 0x00af9a000000ffff  # CODE SEGMENT64
  .quad 0x00af92000000ffff  # DATA SEGMENT64
gdt_desc64:
  .word 0x17
  .quad RELOC(gdt64)

.p2align 12
.globl boot_pml4e
.globl boot_pdpt1
.globl boot_pdpt2
.globl boot_pde1
.globl boot_pde2

boot_pml4e:
  .space  0x1000
boot_pdpt1:
  .space  0x1000
boot_pdpt2:
  .space  0x1000
boot_pde1:
  .space  0x1000
boot_pde2:
  .space  0x1000

.section .text
.code64
.globl entry_64
.func entry_64
entry_64:
	#### 0 ~ 0x1000 영역을 부트 스택으로 사용한다.
	xor %rbp, %rbp
	movabs $(LOADER_KERN_BASE + 0x1000), %rsp
	#### 이 지점부터 CPU는 임시 부트 페이지 테이블과 스택을 가진 64비트
	#### long mode 상태다. main() 호출은 C 레벨 커널 초기화로 들어가는
	#### 경계이며, 실제 할당자, 페이지 테이블, 인터럽트 테이블, 스케줄러,
	#### 선택적인 user/file/VM 하위 시스템은 아직 여기서부터 만들어야 한다.
	movabs $main, %rax
	call *%rax
.endfunc
