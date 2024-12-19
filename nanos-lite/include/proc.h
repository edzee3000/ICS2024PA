#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)
#define STACK_PAGE_NUM 8


typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()  
    // 我们不free释放内存所以要使用max_brk成员函数取决定什么时候调用map函数
    // 为了在分页机制上运行仙剑奇侠传, 我们还需要考虑堆区的问题. 之前我们让mm_brk()函数直接返回0, 
    // 表示用户进程的堆区大小修改总是成功, 这是因为在实现分页机制之前, 0x3000000/0x83000000之上的内存都可以让用户进程自由使用. 
    // 现在用户进程运行在分页机制之上, 我们还需要在mm_brk()中把新申请的堆区映射到虚拟地址空间中, 
    // 这样才能保证运行在分页机制上的用户进程可以正确地访问新申请的堆区.
    // 为了识别堆区中的哪些空间是新申请的, 我们还需要记录堆区的位置. 由于每个进程的堆区使用情况是独立的, 
    // 我们需要为它们分别维护堆区的位置, 因此我们在PCB中添加成员max_brk, 来记录program break曾经达到的最大位置.
    //  引入max_brk是为了简化实现: 我们可以不实现堆区的回收功能, 而是只为当前新program break超过max_brk部分的虚拟地址空间分配物理页.
    uintptr_t max_brk; 
  };
} PCB;

extern PCB *current;

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) ;
Context* schedule(Context *prev);
void switch_boot_pcb() ;

int mm_brk(uintptr_t brk);

#endif
