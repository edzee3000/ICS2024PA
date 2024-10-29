#ifndef __ETRACE_H__
#define __ETRACE_H__


#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
#include <device/map.h>

extern CPU_state cpu;
void etrace_errors(word_t NO, vaddr_t epc);



#endif