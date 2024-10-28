#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

struct Context {
  // TODO: fix the order of these members to match trap.S   修正成员的顺序以匹配 trap.S 文件
  // 重新组织Context结构体，观察trap.S中将参数保存到栈中的顺序，调整Context内的字段的声明顺序与保存顺序对应即可
  // uintptr_t mepc, mcause, gpr[NR_REGS], mstatus;  这个是原本的顺序
  uintptr_t gpr[NR_REGS], mcause, mstatus, mepc;
  void *pdir;
  // uintptr_t np;
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7   #####################################放在a7里面！！！！！！！！！！！！！
#endif

#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
