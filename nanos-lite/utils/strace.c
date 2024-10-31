#include <strace.h>
#include <src/syscall.h>

void System_Trace(Context* c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  printf("STRACE得到系统调用信息:\t");
  switch (a[0]) {
    //你需要实现SYS_exit系统调用（case 0的情况）, 它会接收一个退出状态的参数. 为了方便测试, 我们目前先直接使用这个参数调用halt().    halt(0)表示成功退出 其余均为失败退出
    case SYS_exit: printf("系统调用编号:%d\t系统调用:SYS_exit\t返回值:c->GPRx=0\n",a[0]);  break;//对于c->mcause=1的情况，查看navy-apps/libs/libos/src/syscall.h对应为SYS_exit系统退出
    case SYS_yield:printf("系统调用编号:%d\t系统调用:SYS_yield\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;  //c->mcause为系统调用SYS_yield的情况
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

