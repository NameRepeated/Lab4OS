#include "printk.h"
#include "sbi.h"

extern void test();

int start_kernel() {
    printk("Hello RISC-V\n");
    printk("idle process is running!\n");

    test();

	return 0;
}
