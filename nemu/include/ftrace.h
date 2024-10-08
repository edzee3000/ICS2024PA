#ifndef __FTRACE_H__
#define __FTRACE_H__

// Located at src/isa/$(GUEST_ISA)/include/isa-def.h

#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>

typedef struct {
	char name[32]; // 函数名,32字节应该足够了
	vaddr_t addr;
	unsigned char info;
	Elf32_Xword size;
} SymEntry;

void trace_func_call(vaddr_t pc, vaddr_t target) ;


#endif