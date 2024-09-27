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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, TYPE_B, TYPE_J// none
};

//框架代码定义了src1R()和src2R()两个辅助宏, 用于寄存器的读取结果记录到相应的操作数变量中
//框架代码还定义了immI等辅助宏, 用于从指令中抽取出立即数(对立即数进行赋值)

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = (SEXT(BITS(i,31,31),1)<<19) | (SEXT(BITS(i,19,12),8)<<11) | (SEXT(BITS(i,20,20),1)<<10) | BITS(i,30,21);} while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  //decode_operand中用到了宏BITS和SEXT, 它们均在nemu/include/macro.h中定义, 分别用于位抽取和符号扩展
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  //decode_operand会首先统一对目标操作数进行寄存器操作数的译码, 即调用*rd = BITS(i, 11, 7), 不同的指令类型可以视情况使用rd
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;//这里好像需要自己手动添加J型指令的宏命令
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}
//我们还是不知道操作对象(比如立即数是多少, 写入到哪个寄存器). 为了解决这个问题, 代码需要进行进一步的译码工作, 这是通过调用decode_operand()函数来完成的.
//decode_operand()函数将会根据传入的指令类型type来进行操作数的译码, 译码结果将记录到函数参数rd, src1, src2和imm中, 它们分别代表目的操作数的寄存器号码, 两个源操作数和立即数


//其中INSTPAT(意思是instruction pattern)是一个宏(在nemu/include/cpu/decode.h中定义), 
//它用于定义一条模式匹配规则. 其格式如下:
//INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);//例如对于auipc指令, 由于译码阶段已经把U型立即数记录到操作数imm中了, 我们只需要通过R(rd) = s->pc + imm将立即数与当前PC值相加并写入目标寄存器中, 这样就完成了指令的执行.
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  
  
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd)=(imm<<12));//########################################lui和li命令待定  这里好像那个有点问题
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, {R(rd) = src1 + imm;});
  //INSTPAT("??????? ????? ????? ??? ????? 00100 11",li,I,LI_Inst());
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));//将src2数据存储到src1+imm的位置上去，并且写入的字节数为4
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, {R(rd)=s->snpc; s->dnpc=(s->pc)+imm*2;});//注意在inst_fetch当中已经实现了pc+4的过程，pc没有发生改变，但是snpc和dnpc都已经变化了     注意这里之前还出了一个bug就是有关于地址运算需要+imm*4??????????
  
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));//在模式匹配过程的最后有一条inv的规则, 表示"若前面所有的模式匹配规则都无法成功匹配, 则将该指令视为非法指令
  INSTPAT_END();
/*
·模式字符串中只允许出现4种字符:
0表示相应的位只能匹配0
1表示相应的位只能匹配1
?表示相应的位可以匹配0或1
空格是分隔符, 只用于提升模式字符串的可读性, 不参与匹配
·指令名称在代码中仅当注释使用, 不参与宏展开;
·指令类型用于后续译码过程; 
·而指令执行操作则是通过C代码来模拟指令执行的真正行为.
*/

  R(0) = 0; // reset $zero to 0

  return 0;//指令执行的阶段结束之后, decode_exec()函数将会返回0, 并一路返回到exec_once()函数中. 不过目前代码并没有使用这个返回值, 因此可以忽略它.
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}




// void LI_Inst()
// {

// }
