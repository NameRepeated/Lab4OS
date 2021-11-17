.extern mm_init
.extern start_kernel
.extern sbi_set_timer
.extern task_init

    .section .text.init
    .globl _start
_start:
    la sp, boot_stack_top # 将预留的栈空间写入 $sp

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

    la sp, boot_stack_top
    call mm_init
    
    call task_init

    call start_kernel # 跳转至 main.c 中的 start_kernel 函数

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 将栈大小设置为 4KB

    .globl boot_stack_top
boot_stack_top:
    # 栈结尾