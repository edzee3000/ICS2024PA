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
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) ;
Context* schedule(Context *prev);
void switch_boot_pcb() ;

#endif
