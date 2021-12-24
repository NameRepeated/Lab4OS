#include "printk.h"
#include "sbi.h"

extern void test();

extern void schedule();

int start_kernel() {
    printk("[S-MODE] Hello RISC-V\n");
    // printk("idle process is running!\n");

    schedule();

    test();

	return 0;
}
