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

#include <isa.h>
#include "local-include/reg.h"

/*这行代码定义了一个字符串数组 regs，包含了寄存器的名称。
* 数组中的每个元素都是一个指向 const char 的指针，指向一个字符串，
* 这些字符串代表了RISC-V架构中的寄存器名。
* 例如，"$0" 是零寄存器，"ra" 是返回地址寄存器，"sp" 是栈指针寄存器，等等。
*/
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

//在sdb.c当中调用了isa_reg_display函数接口，而具体函数实现在这里实现
//在reg.h的头文件当中有出现过cpu.gpr[check_reg_idx(idx)]，应该是表明cpu.gpr是cpu的通用目的寄存器组（结合一下全称发现还真是）
//接下来只需要
//查看isa.h的头文件发现这里会根据CONFIG的不同而采取不一样的gpr和pc
//由于本机采用的是riscv32位指令集体系架构，因此对应的是CONFIG_ISA_riscv
//对应的gprs为 uint32_t gpr[32];
void isa_reg_display() {
  int num_regs=sizeof(regs) / sizeof(regs[0]);
  for(int idx=0;idx<num_regs;idx++)
  {
    const char *name_reg = reg_name(idx);
    uint32_t value=gpr(idx);
    //uint32_t addre=(gpr(idx));
    //printf("%s\t%#X\t%#X\n" , name_reg,addre, value);
    printf("%s\t%#X\n" , name_reg, value);
  }

}

word_t isa_reg_str2val(const char *s, bool *success) {
  return 0;
}
