#include "defs.h"
#include "mm.h"
#include "printk.h"
#include "string.h"

#define VPN2(va) ((va >> 30) & 0x1ff)
#define VPN1(va) ((va >> 21) & 0x1ff)
#define VPN0(va) ((va >> 12) & 0x1ff)

extern unsigned long swapper_pg_dir[512];

void linear_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, int perm);

void setup_vm(void);

void create_mapping(unsigned long *root_pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm);

void setup_vm_final(void);