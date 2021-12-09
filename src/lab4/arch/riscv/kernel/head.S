#include "defs.h"

.extern mm_init
.extern start_kernel
.extern sbi_set_timer
.extern task_init
.extern early_pgtbl
.extern setup_vm
.extern setup_vm_final

    .section .text.init
    .globl _start
_start:
    la sp, boot_stack_top # 将预留的栈空间写入 $sp

    call setup_vm
    call relocate

    call mm_init

    call setup_vm_final

    # set stvec = _traps
    la t0, _traps
    csrw stvec, t0

    # set sie[STIE] = 1
    li t0, 1<<5 # STIE 在 sie 的第五比特位
    csrs sie, t0

    # set first time interrupt
    rdtime a0
    li t0, 10000000
    add a0, a0, t0
    call sbi_set_timer

    # set sstatus[SIE] = 1
    csrsi sstatus, 1<<1  # SIE 在 sstatus 的第一比特位
    
    call task_init

    call start_kernel # 跳转至 main.c 中的 start_kernel 函数

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    li t0, PA2VA_OFFSET
    add ra, ra, t0
    add sp, sp, t0

    # set satp with early_pgtbl
    li t0, 8
    slli t0, t0, 60
    la t1, early_pgtbl
    srli t1, t1, 12
    add t0, t0, t1
    csrw satp, t0
    
    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 将栈大小设置为 4KB

    .globl boot_stack_top
boot_stack_top:
    # 栈结尾