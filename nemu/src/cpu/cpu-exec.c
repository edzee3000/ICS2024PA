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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen  当执行的指令数量小于这个值时，执行的指令汇编代码才会输出到屏幕上。
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.当你使用 `si' 命令时，这很有用。
 * You can modify this value as you want.你可以根据自己的需要修改这个值。
 */
#define MAX_INST_TO_PRINT 10//最多打印的指令数目

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();
void wp_diff_test() ;


static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));


// #ifdef CONFIG_WATCHPOINT
  // wp_diff_test();  //***************************在pa2当中为了提高效率暂时先把watchpointer的功能给关闭掉********************************************
  // IFDEF(CONFIG_WATCHPOINT, wp_diff_test()); //使用CONFIG_WATCHPOINT的宏把检查监视点的代码包起来
  //后在nemu/Kconfig中为监视点添加一个开关选项, 最后通过menuconfig打开这个选项, 从而激活监视点的功能
// #endif
  
}


//这个函数执行单个指令，并更新CPU状态。它还负责调用 trace_and_difftest 函数来记录执行的指令，以便进行跟踪和差异测试。
//exec_once()函数覆盖了指令周期的所有阶段: 取指, 译码, 执行, 更新PC
static void exec_once(Decode *s, vaddr_t pc) {
  //exec_once()会先把当前的PC保存到s的成员pc和snpc中, 
  //其中s->pc就是当前指令的PC, 而s->snpc则是下一条指令的PC, 这里的snpc是"static next PC"的意思.
  s->pc = pc;
  s->snpc = pc;
  //然后代码会调用isa_exec_once()函数(在nemu/src/isa/$ISA/inst.c中定义), 
  //这是因为执行指令的具体过程是和ISA相关的, 在这里我们先不深究isa_exec_once()的细节. 
  //但可以说明的是, 它会随着取指的过程修改s->snpc的值, 使得从isa_exec_once()返回后s->snpc正好为下一条指令的PC.
  isa_exec_once(s);
  //接下来代码将会通过s->dnpc来更新PC, 这里的dnpc是"dynamic next PC"的意思. 关于snpc和dnpc的区别, 我们会在下文进行说明.
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#endif
}

//这个函数接受一个整数 n，表示要执行的指令数量。它循环调用 exec_once 函数来执行每一条指令。
//PA1中提到：代码将在一个for循环中不断调用exec_once()函数, 这个函数的功能就是我们在上一小节中介绍的内容: 让CPU执行当前PC指向的一条指令, 然后更新PC.
static void execute(uint64_t n) {
  Decode s;//Decode类型的结构体指针s，用于存放在执行一条指令过程中所需的信息，包括指令的PC与下一条指令的PC、与ISA相关的信息
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works.模拟CPU如何运行 */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
