#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  pf += nr_page * PGSIZE;//来自于 nanos-lite/include/memory.h  #define PGSIZE 4096   4KB
  return pf;  //它会通过一个pf指针来管理堆区, 用于分配一段大小为nr_page * 4KB的连续内存区域, 并返回这段区域的首地址(????)
  // return NULL;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {//page页分配策略
  //首先实现pg_alloc()，pg_alloc()的参数是分配空间的字节数，调用 new_page 分配 n 字节大小的内存，并返回这块内存的起始地址，这个起始地址用作页目录的基地址
  int nr_page = n / PGSIZE;//计算要分配page页的个数
  assert(nr_page * PGSIZE == n);//如果n不是PGSIZE的整数倍的话则要asser(0)报错
  void *end = new_page(nr_page);
  void *start = end - n;//计算起始位置start的地址
  memset(start, 0, n);//此外pg_alloc()还需要对分配的页面清零.
  return start;
  // return NULL;
}
#endif

void free_page(void *p) {//page页回收策略
  panic("not implement yet");
}

#define NEW_PAGE_ADDR(nr_page) (new_page(nr_page) - nr_page * PGSIZE)
#define PG_MASK (~0xfff)  //页对齐 4KB大小
#define ISALIGN(vaddr) ((vaddr) == ((vaddr)&PG_MASK))
#define OFFSET(vaddr) ((vaddr) & (~PG_MASK))
#define NEXT_PAGE(vaddr) ((ISALIGN(vaddr)) ? (vaddr) : ((vaddr)&PG_MASK) + PGSIZE)
/* The brk() system call handler. brk函数系统调用处理*/
int mm_brk(uintptr_t brk) {
  //根据上述内容, 实现nanos-lite/src/mm.c中的mm_brk()函数. 你需要注意map()参数是否需要按页对齐的问题(这取决于你的map()实现).  
  //注意我的map函数里面是要求进行页对齐的因此这里需要小心
  if (current->max_brk == 0)
  {
    current->max_brk = NEXT_PAGE(brk);//如果brk是页对齐的则返回brk  如果brk不是页对齐的则返回下一个页面的首地址
    printf("首次分配current->max_brk的值为: %x\n", (void *)current->max_brk);
    return 0;
  }
  for (; current->max_brk < brk; current->max_brk += PGSIZE)
  {
  #ifdef HAVE_PAGE
    void* pa=pg_alloc(PGSIZE); 
    void* va=(void *)current->max_brk;
    // va= (void*)(((uintptr_t)(va)) & PG_MASK);//注意这个时候是需要进行强制类型转换的！！！
    // printf("va的值为: %x\n",va);
    map(&current->as, va, pa , PTE_R | PTE_W | PTE_X);
  #endif
  }
  return 0;
}

void init_mm() {
  //目前初始化MM的工作有两项, 第一项工作是将TRM提供的堆区起始地址作为空闲物理页的首地址, 
  // 这样以后, 将来就可以通过new_page()函数来分配空闲的物理页了. 为了简化实现, MM可以采用顺序的方式对物理页进行分配,
  //  而且分配后无需回收. 第二项工作是调用AM的vme_init()函数. 以riscv32为例,
  //  vme_init()将设置页面分配和回收的回调函数, 然后调用map()来填写内核虚拟地址空间(kas)的页目录和页表,
  //  最后设置一个叫satp(Supervisor Address Translation and Protection)的CSR寄存器来开启分页机制. 
  // 这样以后, Nanos-lite就运行在分页机制之上了.
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);//定义在abstract-machine/am/src/riscv/nemu/vme.c当中
#endif
}
