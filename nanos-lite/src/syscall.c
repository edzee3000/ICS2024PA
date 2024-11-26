#include <common.h>
#include "syscall.h"

#include <fs.h>
#include <proc.h>

#include <sys/time.h>
#include <time.h>

#include <declaration.h>

void naive_uload(PCB *pcb, const char *filename);//在nanos-lite/src/loader.c里面定义的函数
// #include "files.h" //用于strace的翻译文件名
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

int system_open(const char *pathname, int flags, int mode);
int system_close(int fd);
size_t system_lseek(int fd, size_t offset, int whence);
size_t system_read(int fd, intptr_t buf, size_t count);
int system_gettimeofday(struct timeval *tv, struct timezone *tz);
int system_execve(const char *pathname, char *const _Nullable argv[],char *const _Nullable envp[]);



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
    case SYS_exit: c->GPRx=0;/*printf("do_syscall(0)\tSYS_exit\t返回值c->GPRx=%d\n",c->GPRx);*/ 
                  if(c->GPRx==0){system_execve("/bin/menu",NULL,NULL);}//有了开机菜单程序之后, 就可以很容易地实现一个有点样子的批处理系统了. 你只需要修改SYS_exit的实现, 让它调用SYS_execve来再次运行/bin/menu, 而不是直接调用halt()来结束整个系统的运行. 这样以后, 在一个用户程序结束的时候, 操作系统就会自动再次运行开机菜单程序, 让用户选择一个新的程序来运行.
                  else{halt(c->GPRx);}//对于c->mcause=1的情况，查看navy-apps/libs/libos/src/syscall.h对应为SYS_exit系统退出
                  break;
    case SYS_yield:/*printf("do_syscall(1)\tSYS_yield\t返回值c->GPRx=%d\n",c->GPRx);*/
                  yield(); break;  //c->mcause为系统调用SYS_yield的情况
    case SYS_write:  c->GPRx = system_write(a[1],  a[2] , a[3]);
                  /*printf("do_syscall(4)\tSYS_write\tfd=%d\tbuf=%d\tcount=%u\t返回值c->GPRx=%d\n",a[1],a[2],a[3],c->GPRx);*/ break;//返回值为写入的字节数。
    case SYS_brk: c->GPRx = system_brk(a[1]); //接收一个参数addr, 用于指示新的program break的位置. 
                  /*printf("do_syscall(9)\tSYS_brk\t返回值c->GPRx=%d\n",c->GPRx);*/ break;
    case SYS_open:c->GPRx = system_open((const char *)a[1],  a[2] , a[3]);/*printf("调用SYS_open\n");*/break;
    case SYS_close:c->GPRx = system_close(a[1]);/*printf("调用SYS_close\n");*/break;
    case SYS_read:c->GPRx = system_read(a[1],  a[2] , a[3]);/*printf("调用SYS_read\n");*/break;
    case SYS_lseek:c->GPRx = system_lseek(a[1],  a[2] , a[3]);/*printf("调用SYS_lseek\n");*/break;
    case SYS_gettimeofday:c->GPRx = system_gettimeofday((struct timeval *)a[1],  (struct timezone *)a[2]);break;
    case SYS_execve: panic("执行到了execve"); c->GPRx =system_execve((const char *)a[1],  (char *const *)a[2] ,  (char *const *)a[3]);break;
    // case SYS_fb_write:c->GPRx = FB_write(a[1],  a[2] , a[3]);break;

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
  // 1. program break一开始的位置位于_end
  // 2. 被调用时, 根据记录的program break位置和参数increment, 计算出新program break
  // 3. 通过SYS_brk系统调用来让操作系统设置新program break
  // 4. 若SYS_brk系统调用成功, 该系统调用会返回0, 此时更新之前记录的program break的位置, 并将旧program break的位置作为_sbrk()的返回值返回
  // 5. 若该系统调用失败, _sbrk()会返回-1





}




int system_open(const char *pathname, int flags, int mode)
{ return fs_open(pathname,flags,mode);}

int system_close(int fd)
{return fs_close(fd);}

size_t system_lseek(int fd, size_t offset, int whence)
{return fs_lseek(fd,offset,whence);}

size_t system_write(int fd, intptr_t buf, size_t count)
{//这里会出一个问题  就是这个buf在调试的时候显示的是0  ？？？？？？？
  // if(fd!=1&&fd!=2) return -1;
  
  // assert(fd==1||fd==2); //这里后来也出了问题  如果有了fs_write了之后就不需要assert了，因为有一个忽略标准输入的判断语句在fs_write当中
  
  // char* ptr=(char *)buf;
  // for(int i=0;i<count;i++){putch(ptr[i]);}
  // return count;
  // if(fd==3){printf("insert fd==3\n");}

  
  return fs_write(fd,(const void*)buf,count);
}


size_t system_read(int fd, intptr_t buf, size_t count)
{return fs_read(fd,(void*)buf,count);}


//############################这里我还不太确定是用increment还是用program_break去作为参数#################################
intptr_t system_brk(intptr_t increment)
{return 0;//暂时先返回0
}

int system_gettimeofday(struct timeval *tv, struct timezone *tz) 
{
  // // tv：对于gettimeofday，指向存放返回的时间信息的缓冲区；对于settimeofday，指向需要设置的时间信息缓冲区。原型如下
  // struct timeval {
  //     time_t      tv_sec;     /* 秒 */
  //     suseconds_t tv_usec;    /* 微妙 */
  // };
  // // tz：时区信息，一般不会被使用。原型如下
  // struct timezone {
  //     int tz_minuteswest;     /* minutes west of Greenwich */
  //     int tz_dsttime;         /* type of DST correction */
  // };
  // 返回说明：
  // 成功执行时，返回0。失败返回-1，errno被设为以下的某个值
  // EFAULT：tv或tz其中某一项指向的空间不可访问
  // EINVAL：时区格式无效
  // EPERM：权限不足，调用进程不允许使用settimeofday设置当前时间和时区值。
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  if(tv!=NULL){
  tv->tv_sec = us / (1000*1000);//秒
  tv->tv_usec = us %(1000*1000);}//微秒
  if(tz!=NULL){}//时区信息一般不会被使用  因此这里先暂时不实现了吧
  return 0;
}


int system_execve(const char *pathname, char *const _Nullable argv[],char *const _Nullable envp[])
{
  //  execve() executes the program referred to by pathname.  This causes the program
  //  that  is  currently  being run by the calling process to be replaced with a new
  //  program, with newly initialized stack, heap, and  (initialized  and  uninitial‐
  //  ized) data segments.
  
  //execve() does not return on success, and the text, initialized data, uninitial‐
      //  ized data (bss), and stack of the calling process are overwritten according  to
      //  the contents of the newly loaded program.
  
  int fd = fs_open(pathname, 0, 0);
  if (fd == -1)  return -1;
  else  fs_close(fd);
  printf("准备运行程序:%s\n",pathname);
  naive_uload(NULL,pathname);
  yield();

  return 0;
  
  
  // 在PA3的最后, 你将会向Nanos-lite中添加一些简单的功能, 来展示你的批处理系统.
  // 你之前已经在Navy上执行了开机菜单和NTerm, 但它们都不支持执行其它程序. 这是因为"执行其它程序"需要一个新的系统调用来支持, 
  // 这个系统调用就是SYS_execve, 它的作用是结束当前程序的运行, 并启动一个指定的程序. 这个系统调用比较特殊, 如果它执行成功, 
  // 就不会返回到当前程序中, 具体信息可以参考man execve. 为了实现这个系统调用, 
  // 你只需要在相应的系统调用处理函数中调用naive_uload()就可以了. 目前我们只需要关心filename即可, 
  // argv和envp这两个参数可以暂时忽略.

  //你需要实现SYS_execve系统调用, 然后通过开机菜单来运行其它程序. 你已经实现过很多系统调用了, 需要注意哪些细节, 这里就不啰嗦了.
}










//#####################################################strace功能的实现############################################################################
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
} MyFileInfo;

static MyFileInfo my_define_file_table[] __attribute__((used)) = {
  [0]  = {"stdin", 0, 0},
  [1] = {"stdout", 0, 0},
  [2] = {"stderr", 0, 0},
#include "files.h"  //nanos-lite/src/files.h包含进来表文件列表
};
//strace代码放在这里好了  因为新开一个新的strace.c文件出问题了最后……
void System_Trace(Context* c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  printf("STRACE:\t");
  switch (a[0]) {
    //你需要实现SYS_exit系统调用（case 0的情况）, 它会接收一个退出状态的参数. 为了方便测试, 我们目前先直接使用这个参数调用halt().    halt(0)表示成功退出 其余均为失败退出
    case SYS_exit: printf("系统调用编号:%d\t系统调用:SYS_exit\t返回值:c->GPRx=0\n",a[0]);  break;//对于c->mcause=1的情况，查看navy-apps/libs/libos/src/syscall.h对应为SYS_exit系统退出
    case SYS_yield:printf("系统调用编号:%d\t系统调用:SYS_yield\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;  //c->mcause为系统调用SYS_yield的情况
    case SYS_write:printf("系统调用编号:%d\t系统调用:SYS_write\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;
    case SYS_brk:  printf("系统调用编号:%d\t系统调用:SYS_brk\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;
    case SYS_open: printf("系统调用编号:%d\t系统调用:SYS_open\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;
    case SYS_close:printf("系统调用编号:%d\t系统调用:SYS_close\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;
    case SYS_read: printf("系统调用编号:%d\t系统调用:SYS_read\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;
    case SYS_lseek:printf("系统调用编号:%d\t系统调用:SYS_lseek\t返回值:c->GPRx=%d\n",a[0], c->GPRx); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  // 由于sfs的特性, 打开同一个文件总是会返回相同的文件描述符. 这意味着, 我们可以把strace中的文件描述符直接翻译成文件名, 得到可读性更好的trace信息. 
  // 尝试实现这一功能, 它可以为你将来使用strace提供一些便利.
  // 观察navy-apps/libs/libos/src/syscall.c当中所有的_syscall_调用函数的第二个参数（a0寄存器 GPR2宏命令）都是fd文件标识符 
  // 目前而言，只有SYS_read SYS_close SYS_lseek SYS_write SYS_open这些系统调用使用到了sys简易文件系统  因而只有这些需要进行文件描述符翻译成文件名
  const char *syscall_name="";
  const char *file_name="";
  switch (a[0])
  {
  case SYS_open:syscall_name="SYS_open";
  case SYS_read:syscall_name="SYS_read";
  case SYS_write:syscall_name="SYS_write";
  case SYS_lseek:syscall_name="SYS_lseek";
  case SYS_close:syscall_name="SYS_close";
    if(a[1] > 2 && a[1]<sizeof(my_define_file_table)/sizeof(MyFileInfo))
    {file_name=my_define_file_table[a[1]].name;
    Log("\n对文件%s进行%s文件操作",file_name,syscall_name);}
    break;
  default:break;
  }



}
//#######################################################################################################














