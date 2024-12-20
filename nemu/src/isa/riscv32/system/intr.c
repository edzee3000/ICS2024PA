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
#include <etrace.h>

#define IRQ_TIMER 0x80000007  // for riscv32
extern CPU_state cpu;
word_t isa_raise_intr(word_t NO, vaddr_t epc) {  //触发了一个异常/中断 asm volatile("li a7, -1; ecall");  其中NO为-1存储咋a7当中
  /* TODO: Trigger an interrupt/exception with ``NO''.    触发一个带有“NO”的中断/异常  
   * Then return the address of the interrupt/exception vector.  然后返回中断/异常向量的地址
   */
  //如果会抛出一个异常/中断的话，那调用指令的时候就会调用ecall指令，而且调用ecall指令是一定会调用isa_raise_intr函数的，那么估计etrace功能就应该是放在这里的

  #ifdef CONFIG_ETRACE
  etrace_errors(NO, epc);
  #endif
#ifdef TIME_INTR
  // 保存当前的 MIE(mstatus的第3位) 到 MPIE(mstatus的第7位)  注意还有第0位
  word_t MIE=MIE_VALUE;
  cpu.CSRs.mstatus = (cpu.CSRs.mstatus & ~(1 << MPIE_OFFSET)) | (MIE << MPIE_OFFSET) ;
  // 将 MIE 置为 0
  cpu.CSRs.mstatus &= ~(1 << MIE_OFFSET);
#endif

  // printf("NO的值为:%d\n",NO);
  switch (NO)
  {case -1:     //表示这个时候是需要加4的  因为是yield自陷  看asm手动插入的那一条汇编语言代码
  case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 10:
  case 11:case 12:case 13:case 14:case 15:case 16:case 17:case 18:case 19:  //先暂时默认所有的都需要+4   要是之后有需要调整的再修改……这个一开始只有case0和1结果又报错了……唉……无语死了
  epc+=4; break;   //当case为0和1的时候同样需要epc+=4否则又重新陷入ecall了……
#ifdef TIME_INTR
  case IRQ_TIMER: break;//如果对应的是IRQ_TIMER 0x80000007 错误号的话那我就可以不用epc+4了  这样也就减免了cpu-exec当中的epc-4的操作
#endif
  default: panic("NO错误号无定义\n");  break;}

  cpu.CSRs.mcause=NO;//存储触发异常的原因NO
  cpu.CSRs.mepc=epc; //存储触发异常的pc(如果经历了+4的话就是存储mret的时候下一条将要执行的指令)

  return cpu.CSRs.mtvec;//从mtvec寄存器中取出异常入口地址并返回

  //修改isa_raise_intr()中的代码, 让处理器进入关中断状态
  //riscv32 - 将mstatus.MIE保存到mstatus.MPIE中, 然后将mstatus.MIE位置为0
// 你还需要修改mret指令的实现, 将mstatus.MPIE还原到mstatus.MIE中, 然后将mstatus.MPIE位置为1

}

//你还需要修改mret指令的实现, 将mstatus.MPIE还原到mstatus.MIE中, 然后将mstatus.MPIE位置为1
void modify_mret()
{
  // 还原之前保存的 MPIE 到 MIE
  word_t MPIE=MPIE_VALUE;
  cpu.CSRs.mstatus = (cpu.CSRs.mstatus & ~(1 << MIE_OFFSET)) | (MPIE << MIE_OFFSET);
  // 将 MPIE 位置为 1
  cpu.CSRs.mstatus |= (1 << MPIE_OFFSET);
}



word_t isa_query_intr() {
  //MIE开启“且“cpu中断引脚高电平“时, 接收中断 
  if (cpu.INTR && (MIE_VALUE) ) { //如果触发了中断的话那就关闭中断  然后返回IRQ_TIMER表明有硬件中断到来
    cpu.INTR = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
  // return INTR_EMPTY;
}
