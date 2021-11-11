.altmacro
.set REG_SIZE, 8
.set CONTEXT_SIZE, 34

.macro SAVE reg, offset
    sd \reg, \offset*8(sp)
.endm

.macro SAVE_N n
    SAVE x\n, \n
.endm

.macro LOAD reg, offset
    ld \reg, \offset*8(sp)
.endm

.macro LOAD_N n
    LOAD x\n, \n
.endm

    .section .text.entry
    .align 2
    .globl _traps 
_traps:
    # save 32 registers and sepc to stack
    addi sp, sp, -33*8

    SAVE x1, 1
    addi x1, sp, 33*8
    SAVE x1, 2

    .set n, 3
    .rept 29
        SAVE_N %n
        .set n, n + 1
    .endr

    csrr t0, sepc
    SAVE t0, 32

    # call trap_handler
    csrr a0, scause
    csrr a1, sepc
    call trap_handler

    # restore sepc and 32 registers (x2(sp) should be restore last) from stack
    LOAD t0, 32
    csrw sepc, t0

    LOAD x1, 1
    .set n, 3
    .rept 29
        LOAD_N %n
        .set n, n + 1
    .endr

    LOAD x2, 2

    # return from trap
    sret