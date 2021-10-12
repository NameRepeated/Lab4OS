.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # ------------------

        # set stvec = _traps

    # ------------------

        # set sie[STIE] = 1

    # ------------------

        # set first time interrupt

    # ------------------

        # set sstatus[SIE] = 1

    # ------------------

    la sp, boot_stack_top # 将预留的栈空间写入 $sp
    call start_kernel # 跳转至 main.c 中的 start_kernel 函数

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 将栈大小设置为 4KB

    .globl boot_stack_top
boot_stack_top:
    # 栈结尾