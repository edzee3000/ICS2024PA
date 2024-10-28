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

extern CPU_state cpu;
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.    触发一个带有“NO”的中断/异常
   * Then return the address of the interrupt/exception vector.  然后返回中断/异常向量的地址
   */
  cpu.CSRs.mcause=NO;//存储触发异常的原因NO
  cpu.CSRs.mepc=epc; //存储触发异常的pc
  printf("异常入口地址为:%#x\n",cpu.CSRs.mtvec);
  return cpu.CSRs.mtvec;//从mtvec寄存器中取出异常入口地址并返回
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
