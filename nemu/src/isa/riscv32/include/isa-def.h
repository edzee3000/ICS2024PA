/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>



//riscv32提供了一些特殊的系统寄存器, 叫控制状态寄存器(CSR寄存器)
//这里对CSR寄存器进行定义  给cpu的寄存器种类进行扩充
typedef struct {
  vaddr_t mepc;
  word_t mstatus;
  word_t mcause;
  vaddr_t mtvec;//存储 异常入口地址
  word_t satp;  //在 riscv32_CPU_state 结构体要添加 satp 寄存器!!!!!!!!!!!!!!!!!!!
  word_t mscratch;//riscv32提供了一个叫mscratch的CSR寄存器, 专门作为临时寄存器给系统软件使用, 它在硬件的行为上并没有什么特殊之处  
  // 把概念上的ksp映射到mscratch寄存器
} riscv32_CSRs;


//riscv32的cpu结构体
typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
  riscv32_CSRs CSRs;//如果是riscv64的话也是可以用的  但是这里因为自己默认选择的就是riscv32的因此名字可能取得不是很好
  bool INTR;//在cpu结构体中添加一个bool成员INTR.
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);





// #define TIME_INTR  //时钟中断开关（为了之后测试方便这里加了一个开关）



// decode 解码
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

#define MIE_OFFSET 3
#define MPIE_OFFSET 7 
#define MIE_VALUE ((cpu.CSRs.mstatus>>MIE_OFFSET) & 1)
#define MPIE_VALUE ((cpu.CSRs.mstatus>>MPIE_OFFSET) & 1 )

//你需要理解分页地址转换的过程, 然后实现isa_mmu_check()(在nemu/src/isa/$ISA/include/isa-def.h中定义) 
// 和isa_mmu_translate()(在nemu/src/isa/$ISA/system/mmu.c中定义), 你可以查阅NEMU的ISA相关API说明文档来了解它们的行为.
//  另外由于我们不打算实现保护机制, 在isa_mmu_translate()的实现中, 你务必使用assertion检查页目录项和页表项的present/valid位, 
// 如果发现了一个无效的表项, 及时终止NEMU的运行, 否则调试将会非常困难. 这通常是由于你的实现错误引起的, 请检查实现的正确性.
// #define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)   
//检查 satp 寄存器的 MODE 域即可  即最高位是否为1  如果是1的话就返回MMU_TRANSLATE需要进行地址转换  如果是0的话则表示该内存访问可以在物理内存上直接进行
#define isa_mmu_check(vaddr, len, type) ((((cpu.CSRs.satp & 0x80000000) >> 31) == 1) ? MMU_TRANSLATE : MMU_DIRECT)

// 对于函数int isa_mmu_check(vaddr_t vaddr, int len, int type);而言：
// 检查当前系统状态下对内存区间为[vaddr, vaddr + len), 类型为type的访问是否需要经过地址转换. 其中type可能为:
// MEM_TYPE_IFETCH: 取指令
// MEM_TYPE_READ: 读数据
// MEM_TYPE_WRITE: 写数据
// 函数返回值可能为:
// MMU_DIRECT: 该内存访问可以在物理内存上直接进行
// MMU_TRANSLATE: 该内存访问需要经过地址转换
// MMU_FAIL: 该内存访问失败, 需要抛出异常(如RISC架构不支持非对齐的内存访问)


#endif
