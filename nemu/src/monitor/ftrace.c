#include <common.h>
#include <isa.h>
#include <elf.h>
#include <ftrace.h>
#include <memory/paddr.h>
#define ftrace_write log_write

SymEntry *symbol_tbl = NULL; // dynamic allocated
uint32_t symbol_tbl_size = 0;
uint32_t call_depth = 0;


void trace_func_call(paddr_t pc, paddr_t target) {
	if (symbol_tbl == NULL) return;

	++call_depth;

	if (call_depth <= 2) return; // ignore _trm_init & main

	int i = 0;//find_symbol_func(target, true);
	ftrace_write(FMT_PADDR ": %*scall [%s@" FMT_PADDR "]\n",
		pc,
		(call_depth-3)*2, "",
		i>=0?symbol_tbl[i].name:"???",
		target
	);
}


