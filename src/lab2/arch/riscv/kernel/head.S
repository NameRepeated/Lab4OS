.extern start_kernel
.extern sbi_set_timer

    .section .text.init
    .globl _start
_start:
    la sp, boot_stack_top # 将预留的栈空间写入 $sp

    # set stvec = _traps
    la t0, _traps
    csrw stvec, t0

    # set sie[STIE] = 1
    csrr t0, sie
    ori t0, t0, 1<<5  # STIE 在 sie 的第五比特位
    csrw sie, t0

    # set first time interrupt
    addi sp, sp, -8
    sd ra, 0(sp)

    rdtime a0
    li a1, 0xffffff
    add a0, a0, a1
    call sbi_set_timer

    ld ra, 0(sp)
    addi sp, sp, 8

    # set sstatus[SIE] = 1
    csrr t0, sstatus
    ori t0, t0, 1<<1  # SIE 在 sstatus 的第一比特位
    csrw sstatus, t0

    call start_kernel # 跳转至 main.c 中的 start_kernel 函数

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 将栈大小设置为 4KB

    .globl boot_stack_top
boot_stack_top:
    # 栈结尾