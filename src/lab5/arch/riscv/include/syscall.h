#pragma once

#define SYS_WRITE   64
#define SYS_GETPID  172

struct pt_regs {
  unsigned long x[32];
  unsigned long sepc;
  unsigned long sstatus;
};

void syscall(struct pt_regs* regs);