#include <common.h>
#include <isa.h>
#include <elf.h>
#include <dtrace.h>
#include <device/map.h>
#include <memory/paddr.h>

void trace_dread(paddr_t addr, int len, IOMap *map) {
	Log("dtrace: read %s at " FMT_PADDR ",%d",
		map->name, addr, len);
}

void trace_dwrite(paddr_t addr, int len, word_t data, IOMap *map) {
	Log("dtrace: write %s at " FMT_PADDR ",%d with " FMT_WORD ,
		map->name, addr, len, data);
}