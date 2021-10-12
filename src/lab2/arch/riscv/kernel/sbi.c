#include "types.h"
#include "sbi.h"

struct sbiret sbi_ecall(int ext, int fid, uint64 arg0,
			            uint64 arg1, uint64 arg2,
			            uint64 arg3, uint64 arg4,
			            uint64 arg5) 
{
	struct sbiret ret_val;
	register uint64 a7 asm ("a7") = (uint64)(ext);
	register uint64 a6 asm ("a6") = (uint64)(fid);
	register uint64 a0 asm ("a0") = (uint64)(arg0);
	register uint64 a1 asm ("a1") = (uint64)(arg1);
	register uint64 a2 asm ("a2") = (uint64)(arg2);
	register uint64 a3 asm ("a3") = (uint64)(arg3);
	register uint64 a4 asm ("a4") = (uint64)(arg4);
	register uint64 a5 asm ("a5") = (uint64)(arg5);
	
	__asm__ volatile (
		"ecall"
		: "+r" (a0), "+r" (a1)
		: "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
		: "memory"
	);
	ret_val.error = a0;
	ret_val.value = a1;

	return ret_val;
    // struct sbiret ret_val;
    // __asm__ volatile (
    //     "mv x17, %[ext]\n"
    //     "mv x16, %[fid]\n"
	// 	"mv x10, %[arg0]\n"
	// 	"mv x11, %[arg1]\n"
	// 	"mv x12, %[arg2]\n"
	// 	"mv x13, %[arg3]\n"
	// 	"mv x14, %[arg4]\n"
	// 	"mv x15, %[arg5]\n"
	// 	"ecall\n"
    //     "mv %[ret_val], x10\n"
	// 	"mv %[ret_val]+8, x11\n"
    //     : [ret_val] "=r" (ret_val)
    //     : [ext] "r" (ext), [fid] "r" (fid), [arg0] "r" (arg0), [arg1] "r" (arg1), [arg2] "r" (arg2), [arg3] "r" (arg3), [arg4] "r" (arg4), [arg5] "r" (arg5)
    //     : "memory"
    // );
    // return ret_val;
}