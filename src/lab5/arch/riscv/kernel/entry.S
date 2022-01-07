.extern trap_handler
.extern dummy

.altmacro
.set REG_SIZE, 8
.set CONTEXT_SIZE, 34

.macro SAVE reg1, reg2, offset
    sd \reg1, \offset*8(\reg2)
.endm

.macro SAVE_X_N reg, n
    SAVE x\n, \reg, \n
.endm

.macro SAVE_S_N reg, n
    SAVE s\n, \reg, \n
.endm

.macro LOAD reg1, reg2, offset
    ld \reg1, \offset*8(\reg2)
.endm

.macro LOAD_X_N reg, n
    LOAD x\n, \reg, \n
.endm

.macro LOAD_S_N reg, n
    LOAD s\n, \reg, \n
.endm

.macro SWAP_U_S_STACK reg
    csrr \reg, sscratch
    csrw sscratch, sp
    mv sp, \reg
.endm

    .section .text.entry
    .align 2
    .globl _traps 
_traps:
    csrr t0, sscratch
    beq t0, x0, _traps_switch
    SWAP_U_S_STACK t0
_traps_switch:
    # save 32 registers and sepc to stack
    addi sp, sp, -33*8

    SAVE x1, sp, 1
    addi x1, sp, 33*8
    SAVE x1, sp, 2

    .set n, 3
    .rept 29
        SAVE_X_N sp, %n
        .set n, n + 1
    .endr

    csrr t0, sepc
    SAVE t0, sp, 32

    # call trap_handler
    csrr a0, scause
    mv a1, t0
    mv a2, sp
    call trap_handler

    # restore sepc and 32 registers (x2(sp) should be restore last) from stack
    LOAD t0, sp, 32
    addi t0, t0, 4
    csrw sepc, t0

    LOAD x1, sp, 1
    .set n, 3
    .rept 29
        LOAD_X_N sp, %n
        .set n, n + 1
    .endr

    LOAD x2, sp, 2

    csrr t0, sscratch
    beq t0, x0, _traps_end
    SWAP_U_S_STACK t0
_traps_end:
    # return from trap
    sret

    .globl __switch_to
__switch_to:
    # save state to prev process
    addi t0, a0, 40
    sd ra, 0(t0)
    sd sp, 8(t0)

    addi t0, a0, 56
    .set n, 0
    .rept 12
        SAVE_S_N t0, %n
        .set n, n+1
    .endr

    addi t0, a0, 152
    csrr t1, sepc
    sd t1, 0(t0)
    csrr t1, sstatus
    sd t1, 8(t0)
    csrr t1, sscratch
    sd t1, 16(t0)
    csrr t1, satp
    sd t1, 24(t0)

    # restore state from next process
    addi t0, a1, 40
    ld ra, 0(t0)
    ld sp, 8(t0)

    addi t0, a1, 56
    .set n, 0
    .rept 12
        LOAD_S_N t0, %n
        .set n, n+1
    .endr

    addi t0, a1, 152
    ld t1, 0(t0)
    csrw sepc, t1
    ld t1, 8(t0)
    csrw sstatus, t1
    ld t1, 16(t0)
    csrw sscratch, t1
    ld t1, 24(t0)
    csrw satp, t1

    # flush tlb
    sfence.vma zero, zero

    ret

    .globl __dummy
__dummy:
    SWAP_U_S_STACK t0
    sret