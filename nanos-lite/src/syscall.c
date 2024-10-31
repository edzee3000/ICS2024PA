#include <common.h>
#include "syscall.h"





/*本来想在Kconfig里面设置strace的开关的  结果好像没啥用处  因此在这里我手动添加这个CONFIG_STRACE参数
config STRACE
  depends on TRACE && TARGET_NATIVE_ELF && ENGINE_INTERPRETER
  bool "Enable System-Call tracer"
  default y
config STRACE_COND
  depends on STRACE
  string "Only trace System Call when the condition is true"
  default "true"
*/
#define CONFIG_STRACE
void System_Trace(Context* c);




void do_syscall(Context *c) {
  // Nanos-lite收到系统调用事件之后, 就会调出系统调用处理函数do_syscall()进行处理. 
  // do_syscall()首先通过宏GPR1从上下文c中获取用户进程之前设置好的系统调用参数, 
  // 通过第一个参数 - 系统调用号 - 进行分发. 但目前Nanos-lite没有实现任何系统调用, 因此触发了panic
  uintptr_t a[4];
  a[0] = c->GPR1;//c->GPR1里的GPR1为#define GPR1 gpr[17]也就是a7  也就是c->mcause  存储的是规定好的异常号
  //c->GPRx 表示的是 gpr[4]在nemu里面也就是a0寄存器
  #ifdef CONFIG_STRACE
  System_Trace(c);
  #endif
  switch (a[0]) {
    //你需要实现SYS_exit系统调用（case 0的情况）, 它会接收一个退出状态的参数. 为了方便测试, 我们目前先直接使用这个参数调用halt().    halt(0)表示成功退出 其余均为失败退出
    case SYS_exit: c->GPRx=0;printf("do_syscall(0)\tSYS_exit\t返回值c->GPRx=%d\n",c->GPRx); halt(c->GPRx); break;//对于c->mcause=1的情况，查看navy-apps/libs/libos/src/syscall.h对应为SYS_exit系统退出
    case SYS_yield:printf("do_syscall(1)\tSYS_yield\t返回值c->GPRx=%d\n",c->GPRx);yield(); break;  //c->mcause为系统调用SYS_yield的情况
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  
  
}
























//strace代码放在这里好了  因为新开一个新的strace.c文件出问题了最后……
void System_Trace(Context* c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  printf("STRACE:\t");
  switch (a[0]) {
    //你需要实现SYS_exit系统调用（case 0的情况）, 它会接收一个退出状态的参数. 为了方便测试, 我们目前先直接使用这个参数调用halt().    halt(0)表示成功退出 其余均为失败退出
    case SYS_exit: printf("系统调用编号:%d\t系统调用:SYS_exit\t返回值:c->GPRx=0\n",a[0]);  break;//对于c->mcause=1的情况，查看navy-apps/libs/libos/src/syscall.h对应为SYS_exit系统退出
    case SYS_yield:printf("系统调用编号:%d\t系统调用:SYS_yield\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;  //c->mcause为系统调用SYS_yield的情况
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
