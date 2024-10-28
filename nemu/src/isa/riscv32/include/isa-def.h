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
} riscv32_CSRs;


//riscv32的cpu结构体
typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
  riscv32_CSRs CSRs;//如果是riscv64的话也是可以用的  但是这里因为自己默认选择的就是riscv32的因此名字可能取得不是很好
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);



// decode 解码
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
