#ifndef AM_H__
#define AM_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include ARCH_H // this macro is defined in $CFLAGS
                // examples: "arch/x86-qemu.h", "arch/native.h", ...

// Memory protection flags
#define MMAP_NONE  0x00000000 // no access
#define MMAP_READ  0x00000001 // can read
#define MMAP_WRITE 0x00000002 // can write

// Memory area for [@start, @end)
typedef struct {
  void *start, *end;
} Area;

// Arch-dependent processor context   架构依赖的处理器上下文
typedef struct Context Context;





// An event of type @event, caused by @cause of pointer @ref  一个由 @ref 指针的 @cause 引起的 @event 类型的事件。
typedef struct {
  enum {
    EVENT_NULL = 0,
    EVENT_YIELD, EVENT_SYSCALL, EVENT_PAGEFAULT, EVENT_ERROR,
    EVENT_IRQ_TIMER, EVENT_IRQ_IODEV,
  } event;
  uintptr_t cause, ref;
  const char *msg;
} Event;
//其中event表示事件编号, cause和ref是一些描述事件的补充信息, msg是事件信息字符串, 
// 我们在PA中只会用到event枚举类型. 然后, 我们只要定义一些统一的事件编号(上述枚举常量), 让每个架构在实现各自的CTE API时,
//  都统一通过上述结构体来描述执行流切换的原因, 就可以实现切换原因的抽象了.





// A protected address space with user memory @area
// and arch-dependent @ptr  一个受保护的地址空间，具有用户内存 @area 和架构依赖的 @ptr
typedef struct {
  int pgsize;
  Area area;
  void *ptr;
} AddrSpace;

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------- TRM: Turing Machine 图灵机-----------------------
extern   Area        heap;
void     putch       (char ch);
void     halt        (int code) __attribute__((__noreturn__));

// -------------------- IOE: Input/Output Devices 输入输出设备--------------------
bool     ioe_init    (void);
void     ioe_read    (int reg, void *buf);
void     ioe_write   (int reg, void *buf);
#include "amdev.h"

// ---------- CTE: Interrupt Handling and Context Switching 上下文扩展 中断处理和上下文切换----------
bool     cte_init    (Context *(*handler)(Event ev, Context *ctx));
void     yield       (void);
bool     ienabled    (void);
void     iset        (bool enable);
Context *kcontext    (Area kstack, void (*entry)(void *), void *arg);

// ----------------------- VME: Virtual Memory 虚拟内存-----------------------
bool     vme_init    (void *(*pgalloc)(int), void (*pgfree)(void *));
void     protect     (AddrSpace *as);
void     unprotect   (AddrSpace *as);
void     map         (AddrSpace *as, void *vaddr, void *paddr, int prot);
Context *ucontext    (AddrSpace *as, Area kstack, void *entry);

// ---------------------- MPE: Multi-Processing 多线程处理 ----------------------
bool     mpe_init    (void (*entry)());
int      cpu_count   (void);
int      cpu_current (void);
int      atomic_xchg (int *addr, int newval);

#ifdef __cplusplus
}
#endif

#endif
