#include "printk.h"
#include "sbi.h"

extern void test();

int start_kernel() {
    printk("%d Hello RISC-V\n", 2021);

    test();

	return 0;
}
