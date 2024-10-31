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
// #define CONFIG_STRACE
void System_Trace(Context* c);
size_t system_write(int fd, intptr_t buf, size_t count);
intptr_t system_brk(intptr_t increment);  //这里先暂定参数###########


void do_syscall(Context *c) {
  // Nanos-lite收到系统调用事件之后, 就会调出系统调用处理函数do_syscall()进行处理. 
  // do_syscall()首先通过宏GPR1从上下文c中获取用户进程之前设置好的系统调用参数, 
  // 通过第一个参数 - 系统调用号 - 进行分发. 但目前Nanos-lite没有实现任何系统调用, 因此触发了panic
  uintptr_t a[4];
  a[0] = c->GPR1;//c->GPR1里的GPR1为#define GPR1 gpr[17]也就是a7  也就是c->mcause  存储的是规定好的异常号
  //完了好像知道自己问题出在哪里了……这里的c->GPR不是navy-apps/libs/libos/src/syscall.c中的GPR宏定义  而是在abstract-machine/am/include/arch/riscv.h中的……shit啊啊啊啊啊啊啊啊啊  我这debug了好久啊啊啊
  //但是不对啊 观察riscv.h中也一样啊……怎么回事啊啊啊啊啊    
  //啊啊啊啊啊啊啊啊啊啊啊在debug4个小时之后我终于知道自己错在哪里了啊啊啊啊啊  原来是riscv.h中的宏定义我写错了……我……无语了哥们……
  a[1] = c->GPR2;//a0寄存器
  a[2] = c->GPR3;//a1寄存器
  a[3] = c->GPR4;//a2寄存器
  //c->GPRx 表示的是 gpr[4]在nemu里面也就是a0寄存器
  #ifdef CONFIG_STRACE
  System_Trace(c);
  #endif
  // printf("GPR1:%d\tGPR2:%d\tGPR3:%u\tGPR4:%d\n",a[0],c->GPR2, c->GPR3,c->GPR4);
  switch (a[0]) {
    //你需要实现SYS_exit系统调用（case 0的情况）, 它会接收一个退出状态的参数. 为了方便测试, 我们目前先直接使用这个参数调用halt().    halt(0)表示成功退出 其余均为失败退出
    case SYS_exit: c->GPRx=0;printf("do_syscall(0)\tSYS_exit\t返回值c->GPRx=%d\n",c->GPRx); 
                  halt(c->GPRx); break;//对于c->mcause=1的情况，查看navy-apps/libs/libos/src/syscall.h对应为SYS_exit系统退出
    case SYS_yield:printf("do_syscall(1)\tSYS_yield\t返回值c->GPRx=%d\n",c->GPRx);
                  yield(); break;  //c->mcause为系统调用SYS_yield的情况
    case SYS_write:c->GPRx = system_write(a[1],  a[2] , a[3]);
                  /*printf("do_syscall(4)\tSYS_write\tfd=%d\tbuf=%d\tcount=%u\t返回值c->GPRx=%d\n",a[1],a[2],a[3],c->GPRx);*/ break;//返回值为写入的字节数。
    case SYS_brk: c->GPRx = system_brk(a[1]); //接收一个参数addr, 用于指示新的program break的位置. 
                  printf("do_syscall(9)\tSYS_brk\t返回值c->GPRx=%d\n",c->GPRx); break;
                 
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  
  // 输出是通过SYS_write系统调用来实现  
  // ssize_t write(int fd, const void buf[.count], size_t count);
  // 需要在do_syscall()中识别出系统调用号是SYS_write之后, 检查fd的值, 
  // 如果fd是1或2(分别代表stdout和stderr), 则将buf为首地址的len字节输出到串口(使用putch()即可). 
  // 最后还要设置正确的返回值, 否则系统调用的调用者会认为write没有成功执行, 从而进行重试. 
  // 至于write系统调用的返回值是什么, 请查阅man 2 write. 
  // 另外不要忘记在navy-apps/libs/libos/src/syscall.c的_write()中调用系统调用接口函数.
  // 根据man 2 write手册的return value部分： 
  // 当操作成功时，write() 函数返回写入的字节数。如果发生错误，则返回 -1，并设置 errno 以指示错误。
  // 请注意，成功的 write() 调用可能会传输少于 count 指定的字节数。这种部分写入可能因各种原因发生；例如，因为磁盘设备上没有足够的空间来写入所有请求的字节，或者因为一个阻塞的 write() 调用到套接字、管道或类似对象时，在传输了一些但不是所有请求的字节后被信号处理程序中断。在发生部分写入的情况下，调用者可以再次调用 write() 来传输剩余的字节。随后的调用将传输更多的字节，或者可能返回一个错误（例如，如果磁盘现在已满）。
  // 如果 count 为0且 fd 引用的是普通文件，则 write() 可能会在检测到以下错误之一时返回失败状态。如果没有检测到错误，或者没有执行错误检测，则返回 0，不会产生其他效果。如果 count 为零且 fd 引用的不是普通文件，结果未指定。


  //所谓program break, 就是用户程序的数据段(data segment)结束的位置.
  // 在数据段的上面就应该是堆区了嘛  然后通过sbrk()函数动态变化与初始阶段的end之间的区域就可以当成堆区



  // 但如果堆区总是不可用, Newlib中很多库函数的功能将无法使用, 因此现在你需要实现_sbrk()了. 
  // 为了实现_sbrk()的功能, 我们还需要提供一个用于设置堆区大小的系统调用. 
  // 在GNU/Linux中, 这个系统调用是SYS_brk, 它接收一个参数addr, 用于指示新的program break的位置. _sbrk()通过记录的方式来对用户程序的program break位置进行管理, 其工作方式如下:
  // program break一开始的位置位于_end
  // 被调用时, 根据记录的program break位置和参数increment, 计算出新program break
  // 通过SYS_brk系统调用来让操作系统设置新program break
  // 若SYS_brk系统调用成功, 该系统调用会返回0, 此时更新之前记录的program break的位置, 并将旧program break的位置作为_sbrk()的返回值返回
  // 若该系统调用失败, _sbrk()会返回-1


}





size_t system_write(int fd, intptr_t buf, size_t count)
{//这里会出一个问题  就是这个buf在调试的时候显示的是0  ？？？？？？？
  // if(fd!=1&&fd!=2) return -1;
  assert(fd==1||fd==2);
  char* ptr=(char *)buf;
  for(int i=0;i<count;i++){putch(ptr[i]);}
  return count;
}



//这里我还不太确定是用increment还是用program_break去作为参数
intptr_t system_brk(intptr_t increment)
{
  return 0;//暂时先返回0
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
