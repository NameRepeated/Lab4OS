#include "types.h"
#include "sbi.h"

const unsigned long SBI_TIMER_EXT_ID = 0x54494D45;

struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
			            unsigned long arg1, unsigned long arg2,
			            unsigned long arg3, unsigned long arg4,
			            unsigned long arg5) 
{
	struct sbiret ret_val;
	register unsigned long a7 asm ("a7") = (unsigned long)(ext);
	register unsigned long a6 asm ("a6") = (unsigned long)(fid);
	register unsigned long a0 asm ("a0") = (unsigned long)(arg0);
	register unsigned long a1 asm ("a1") = (unsigned long)(arg1);
	register unsigned long a2 asm ("a2") = (unsigned long)(arg2);
	register unsigned long a3 asm ("a3") = (unsigned long)(arg3);
	register unsigned long a4 asm ("a4") = (unsigned long)(arg4);
	register unsigned long a5 asm ("a5") = (unsigned long)(arg5);
	
	__asm__ volatile (
		"ecall"
		: "+r" (a0), "+r" (a1)
		: "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
		: "memory"
	);
	ret_val.error = a0;
	ret_val.value = a1;

	return ret_val;
}

void sbi_set_timer(unsigned long stime_value) {
    sbi_ecall(SBI_TIMER_EXT_ID, SBI_SET_TIMER, stime_value, 0, 0, 0, 0, 0);
}