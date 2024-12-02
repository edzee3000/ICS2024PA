#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
}


// Virtual Memory Extension 虚拟内存管理/拓展

//创建用户进程的上下文则需要一些额外的考量. 在PA3的批处理系统中, 我们在naive_uload()中直接通过函数调用转移到用户进程的代码, 
// 那时候使用的还是内核区的栈, 万一发生了栈溢出, 确实会损坏操作系统的数据, 不过当时也只有一个用户进程在运行, 我们也就不追究了.
//  但在多道程序操作系统中, 系统中运行的进程就不止一个了, 如果让用户进程继续使用内核区的栈, 万一发生了栈溢出, 就会影响到其它进程的运行, 这是我们不希望看到的.
//因此, 和内核线程不同, 用户进程的代码, 数据和堆栈都应该位于用户区, 而且需要保证用户进程能且只能访问自己的代码, 数据和堆栈. 
// 为了区别开来, 我们把PCB中的栈称为内核栈, 位于用户区的栈称为用户栈. 于是我们需要一个有别于kcontext()的方式来创建用户进程的上下文, 
// 为此AM额外准备了一个API ucontext()(在abstract-machine/am/src/nemu/isa/$ISA/vme.c中定义),
Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  //参数as用于限制用户进程可以访问的内存, 我们在下一阶段才会使用, 目前可以忽略它; 
  // kstack是内核栈, 用于分配上下文结构, entry则是用户进程的入口. 由于目前我们忽略了as参数, 
  // 所以ucontext()的实现和kcontext()几乎一样, 甚至比kcontext()更简单: 连参数都不需要传递.
  //用户栈的分配是ISA无关的, 所以用户栈相关的部分就交给Nanos-lite来进行, ucontext()无需处理.
  //  目前我们让Nanos-lite把heap.end作为用户进程的栈顶, 然后把这个栈顶赋给用户进程的栈指针寄存器就可以了.
  Context *cp = (Context *)kstack.end - 1;
  cp->mepc = (uintptr_t)entry; 
  // return NULL;
  return cp;
  // 栈指针寄存器可是ISA相关的, 在Nanos-lite里面不方便处理. 别着急, 还记得用户进程的那个_start吗? 
  // 在那里可以进行一些ISA相关的操作. 于是Nanos-lite和Navy作了一项约定: Nanos-lite把栈顶位置设置到GPRx中,
  //  然后由Navy里面的_start来把栈顶位置真正设置到栈指针寄存器中.
}
