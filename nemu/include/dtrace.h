#ifndef __DTRACE_H__
#define __DTRACE_H__


#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
#include <device/map.h>

void trace_dread(paddr_t addr, int len, IOMap *map);
void trace_dwrite(paddr_t addr, int len, word_t data, IOMap *map);

#endif