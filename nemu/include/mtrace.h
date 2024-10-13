#ifndef __IRINGBUF_H__
#define __IRINGBUF_H__


#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
#include <utils.h>

typedef struct MTRACE_NODE{
  vaddr_t addr;
  word_t content;
  struct MTRACE_NODE *next;
}MTRACE_NODE;

void trace_memory(vaddr_t addr,word_t content);
void print_trace_memory();


#endif