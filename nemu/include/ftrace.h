#ifndef __FTRACE_H__
#define __FTRACE_H__

// Located at src/isa/$(GUEST_ISA)/include/isa-def.h

#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>


// typedef struct {
// 	char name[32]; // 函数名,32字节应该足够了
// 	paddr_t addr;
// 	unsigned char info;
// 	Elf32_Xword size;
// } SymEntry;

// 定义一个结构体来保存函数信息
typedef struct {
    char name[64]; // 假设函数名称不超过64个字符
    Elf32_Addr addr; // 函数起始地址
    Elf32_Word size; // 函数大小
    uint32_t funct_depth; //ftrace中记录函数深度的
} FunctionInfo;

void parse_elf(const char *elf_file);
void print_func_name(const char *elf_file);

#endif