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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

// #include <memory/vaddr.h>
//#include "../include/common.h"

extern const char *regs[];//注意这里是需要从外部reg.c中引入一个const char数组的regs


#define CHECKDIFFPC(p) if (ref_r->p != cpu.p) { \
  printf("Different values of reg " #p ", REF: "FMT_PADDR "DUT: " FMT_PADDR "\n", ref_r->p, cpu.p); \
  return false; \
}
#define CHECKDIFF(p) if (ref_r->p != cpu.CSRs.p) { \
  printf("Different values of reg " #p ", REF: "FMT_PADDR "DUT: " FMT_PADDR "\n", ref_r->p, cpu.CSRs.p); \
  return false; \
}
#define CHECKDIFF_FMT(name, i) if (ref_r->gpr[i] != gpr(i)) { \
  printf("Different values of reg %s, REF: " FMT_WORD "DUT: " FMT_WORD "\n", name, ref_r->gpr[i], gpr(i)); \
  return false; \
}



//isa_difftest_checkregs()函数, 把通用寄存器和PC与从DUT中读出的寄存器的值进行比较. 
//若对比结果一致, 函数返回true; 如果发现值不一样, 函数返回false, 框架代码会自动停止客户程序的运行. 
//特别地, isa_difftest_checkregs()对比结果不一致时, 第二个参数pc应指向导致对比结果不一致的指令, 可用于打印提示信息.
//RTFSC, 从中找出这一顺序, 并检查你的NEMU实现是否已经满足约束.
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if (ref_r->pc != cpu.pc)//如果ref的pc指令地址不等于当前nemu的cpu指令地址
  {
    Log("Different values of the PC! REF: " FMT_PADDR " DUT: " FMT_PADDR , ref_r->pc, cpu.pc);
    return false;
  }
  for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {//遍历ref与nemu每一个寄存器，对其内容进行比较，如果不相等就返回false
    if (ref_r->gpr[i] != gpr(i)) {
      Log("Different values of reg %s! REF: " FMT_WORD " DUT: " FMT_WORD, regs[i], ref_r->gpr[i], gpr(i));
      return false;
    }
  }


  //#######################################################################考虑一下使用
  // int reg_num = ARRLEN(cpu.gpr);
  // for (int i = 0; i < reg_num; i++) {
  //   CHECKDIFF_FMT(regs[i], i);
  // }
  // CHECKDIFFPC(pc); 
  // CHECKDIFF(mstatus);
	// CHECKDIFF(mcause);
  // CHECKDIFF(mepc);
  // CHECKDIFF(mtvec);
  //#######################################################################




  return true;

}
 
void isa_difftest_attach() {
}
