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
  uintptr_t gpr[NR_REGS], mcause, mstatus, mepc, mscratch;
  void *pdir;
  uintptr_t np;//在Context结构体中添加一个新的成员np, 把概念上的c->np映射到它   np指的是next privilege下一个特权级
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7   #####################################放在a7里面，RISC-V Linux上是通过a7这个寄存器来传递系统调用号的.！！！！！！！！！！！！！
#endif

//在abstract-machine/am/include/arch/目录下的相应头文件中实现正确的GPR?宏, 让它们从上下文c中获得正确的系统调用参数寄存器.

//下面这个是写错了的！！！！！！！！！！！！！！！！！！！！！！可是debug了将近4个小时
// #define GPR2 gpr[4]  //观察_syscall_的代码  asm (GPR2) = a0;  而a0正好为gpr[4]
// #define GPR3 gpr[5]  //asm (GPR3) = a1;  a1正好为gpr[5]
// #define GPR4 gpr[6]  //asm (GPR4) = a2;  a2正好为gpr[6]
// #define GPRx gpr[4]  //  GPRx为a0  观察_syscall_的代码，发现是从a0寄存器取得系统调用的返回结果，因此修改$ISA-nemu.h中GPRx的宏定义，将其改成寄存器a0的下标，然后就可以在操作系统中通过c->GPRx根据实际情况设置返回值了
// 处理系统调用的最后一件事就是设置系统调用的返回值. 对于不同的ISA, 系统调用的返回值存放在不同的寄存器中, 
// 宏GPRx用于实现这一抽象, 所以我们通过GPRx来进行设置系统调用返回值即可.

#define GPR2 gpr[10]  //观察_syscall_的代码  asm (GPR2) = a0;  而a0正好为gpr[10]
#define GPR3 gpr[11]  //asm (GPR3) = a1;  a1正好为gpr[11]
#define GPR4 gpr[12]  //asm (GPR4) = a2;  a2正好为gpr[12]
#define GPRx gpr[10]  //  GPRx为a0  观察_syscall_的代码，发现是从a0寄存器取得系统调用的返回结果，因此修改$ISA-nemu.h中GPRx的宏定义，将其改成寄存器a0的下标，然后就可以在操作系统中通过c->GPRx根据实际情况设置返回值了



/*观察navy-apps/libs/libos/src/syscall.c中的代码：
// extract an argument from the macro array
#define SYSCALL  _args(0, ARGS_ARRAY)
#define GPR1 _args(1, ARGS_ARRAY)
#define GPR2 _args(2, ARGS_ARRAY)
#define GPR3 _args(3, ARGS_ARRAY)
#define GPR4 _args(4, ARGS_ARRAY)
#define GPRx _args(5, ARGS_ARRAY)
...
...
#elif defined(__riscv)
#ifdef __riscv_e
# define ARGS_ARRAY ("ecall", "a5", "a0", "a1", "a2", "a0")
#else
# define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
#endif
...
...
可知GPRx为a0
*/


#endif
