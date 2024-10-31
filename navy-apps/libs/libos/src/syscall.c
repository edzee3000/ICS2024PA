#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

// helper macros
#define _concat(x, y) x ## y
#define concat(x, y) _concat(x, y)
#define _args(n, list) concat(_arg, n) list
#define _arg0(a0, ...) a0
#define _arg1(a0, a1, ...) a1
#define _arg2(a0, a1, a2, ...) a2
#define _arg3(a0, a1, a2, a3, ...) a3
#define _arg4(a0, a1, a2, a3, a4, ...) a4
#define _arg5(a0, a1, a2, a3, a4, a5, ...) a5

// extract an argument from the macro array
#define SYSCALL  _args(0, ARGS_ARRAY)
#define GPR1 _args(1, ARGS_ARRAY)
#define GPR2 _args(2, ARGS_ARRAY)
#define GPR3 _args(3, ARGS_ARRAY)
#define GPR4 _args(4, ARGS_ARRAY)
#define GPRx _args(5, ARGS_ARRAY)

// ISA-depedent definitions
#if defined(__ISA_X86__)
# define ARGS_ARRAY ("int $0x80", "eax", "ebx", "ecx", "edx", "eax")
#elif defined(__ISA_MIPS32__)
# define ARGS_ARRAY ("syscall", "v0", "a0", "a1", "a2", "v0")
#elif defined(__riscv)
#ifdef __riscv_e
# define ARGS_ARRAY ("ecall", "a5", "a0", "a1", "a2", "a0")
#else
# define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
#endif
#elif defined(__ISA_AM_NATIVE__)
# define ARGS_ARRAY ("call *0x100000", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_X86_64__)
# define ARGS_ARRAY ("int $0x80", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_LOONGARCH32R__)
# define ARGS_ARRAY ("syscall 0", "a7", "a0", "a1", "a2", "a0")
#else
#error _syscall_ is not implemented
#endif


// 既然我们通过自陷指令来触发系统调用, 那么对用户程序来说, 用来向操作系统描述需求的最方便手段就是使用通用寄存器了, 
// 因为执行自陷指令之后, 执行流就会马上切换到事先设置好的入口, 通用寄存器也会作为上下文的一部分被保存起来. 
// 系统调用处理函数只需要从上下文中获取必要的信息, 就能知道用户程序发出的服务请求是什么了.
intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2) {
  // 会先把系统调用的参数依次放入寄存器中, 然后执行自陷指令. 
  // 由于寄存器和自陷指令都是ISA相关的, 因此这里根据不同的ISA定义了不同的宏, 来对它们进行抽象. 
  // CTE会将这个自陷操作打包成一个系统调用事件EVENT_SYSCALL, 并交由Nanos-lite继续处理.
  // 下面的定义请参考这一条宏定义  # define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
  register intptr_t _gpr1 asm (GPR1) = type;  //使用a7作为系统传递号  比如"li a7, -1; ecall"为了和RISC-V Linux的系统调用参数传递的约定相匹配
  register intptr_t _gpr2 asm (GPR2) = a0;  
  register intptr_t _gpr3 asm (GPR3) = a1;
  register intptr_t _gpr4 asm (GPR4) = a2;
  register intptr_t ret asm (GPRx);  //在riscv32里面GPRx和a0一般是一样的
  asm volatile (SYSCALL : "=r" (ret) : "r"(_gpr1), "r"(_gpr2), "r"(_gpr3), "r"(_gpr4));
  return ret;//回过头来看dummy程序, 它触发了一个SYS_yield系统调用。我们约定, 这个系统调用直接调用CTE的yield()即可, 然后返回0.
  
  // 经过CTE, 执行流会从do_syscall()一路返回到用户程序的_syscall_()函数中. 
  // 代码最后会从相应寄存器中取出系统调用的返回值, 并返回给_syscall_()的调用者, 告知其系统调用执行的情况(如是否成功等).
}

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
  while (1);
}

int _open(const char *path, int flags, mode_t mode) {
  _exit(SYS_open);
  return 0;
}

int _write(int fd, void *buf, size_t count) {
  assert(fd==1 || fd==2);
  _syscall_(SYS_write, (intptr_t)buf, count, 0);  //buf给a0寄存器  count给a1寄存器  0给a2寄存器
  // _syscall_(SYS_write, (intptr_t)buf, count, fd);//如果fd是1或2(分别代表stdout和stderr), 则将buf为首地址的len字节输出到串口(使用putch()即可). 
  _exit(SYS_write);
  return 0;
}

extern char _end;//我们知道可执行文件里面有代码段和数据段, 链接的时候ld会默认添加一个名为_end的符号, 来指示程序的数据段结束的位置. 

//调整堆区大小是通过sbrk()库函数来实现的
void *_sbrk(intptr_t increment) {
  //用于将用户程序的program break增长increment字节, 其中increment可为负数. 
  // 所谓program break, 就是用户程序的数据段(data segment)结束的位置. 
  // 我们知道可执行文件里面有代码段和数据段, 链接的时候ld会默认添加一个名为_end的符号, 来指示程序的数据段结束的位置
  // 用户程序开始运行的时候, program break会位于_end所指示的位置, 意味着此时堆区的大小为0. 
  // malloc()被第一次调用的时候, 会通过sbrk(0)来查询用户程序当前program break的位置, 
  // 之后就可以通过后续的sbrk()调用来动态调整用户程序program break的位置了. 
  // 当前program break和和其初始值之间的区间就可以作为用户程序的堆区, 由malloc()/free()进行管理.
  //  注意用户程序不应该直接使用sbrk(), 否则将会扰乱malloc()/free()对堆区的管理记录
  static intptr_t program_break = (intptr_t)& _end;
  intptr_t original_program_break=program_break;
  if (_syscall_(SYS_brk, increment, 0, 0) == 0) {
    program_break += increment;
    return (void*)original_program_break; 
   }
  return (void *)-1;
  //若SYS_brk系统调用成功, 该系统调用会返回0, 此时更新之前记录的program break的位置, 并将旧program break的位置作为_sbrk()的返回值返回

  //由于目前Nanos-lite还是一个单任务操作系统, 空闲的内存都可以让用户程序自由使用, 
  // 因此我们只需要让SYS_brk系统调用总是返回0即可, 表示堆区大小的调整总是成功. 
  // 在PA4中, 我们会对这一系统调用进行修改, 实现真正的内存分配.

}

int _read(int fd, void *buf, size_t count) {
  _exit(SYS_read);
  return 0;
}

int _close(int fd) {
  _exit(SYS_close);
  return 0;
}

off_t _lseek(int fd, off_t offset, int whence) {
  _exit(SYS_lseek);
  return 0;
}

int _gettimeofday(struct timeval *tv, struct timezone *tz) {
  _exit(SYS_gettimeofday);
  return 0;
}

int _execve(const char *fname, char * const argv[], char *const envp[]) {
  _exit(SYS_execve);
  return 0;
}

// Syscalls below are not used in Nanos-lite.
// But to pass linking, they are defined as dummy functions.

int _fstat(int fd, struct stat *buf) {
  return -1;
}

int _stat(const char *fname, struct stat *buf) {
  assert(0);
  return -1;
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}

pid_t _fork() {
  assert(0);
  return -1;
}

pid_t vfork() {
  assert(0);
  return -1;
}

int _link(const char *d, const char *n) {
  assert(0);
  return -1;
}

int _unlink(const char *n) {
  assert(0);
  return -1;
}

pid_t _wait(int *status) {
  assert(0);
  return -1;
}

clock_t _times(void *buf) {
  assert(0);
  return 0;
}

int pipe(int pipefd[2]) {
  assert(0);
  return -1;
}

int dup(int oldfd) {
  assert(0);
  return -1;
}

int dup2(int oldfd, int newfd) {
  return -1;
}

unsigned int sleep(unsigned int seconds) {
  assert(0);
  return -1;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  assert(0);
  return -1;
}

int symlink(const char *target, const char *linkpath) {
  assert(0);
  return -1;
}

int ioctl(int fd, unsigned long request, ...) {
  return -1;
}
