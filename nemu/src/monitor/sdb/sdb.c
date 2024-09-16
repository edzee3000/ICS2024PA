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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
//由于需要将char*类型的字符串数组转化为int类型数字，因此需要调用atoi函数
#include <stdlib.h>


word_t vaddr_read(vaddr_t addr, int len);////////////////////////为什么在这里进行函数声明就可以了，而不用#include进vaddr.h的头文件呢？



static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);//调用cpu_exec执行完成全部指令，注意在这里-1是unsigned int类型会变为2^32，表示继续往下执行到底（如果不出意外的话）
  return 0;
}


static int cmd_q(char *args) {
	//cpu_exec(NEMU_QUIT);
	nemu_state.state=NEMU_QUIT;
  return -1;
}


//############################# Have Written Code Here ##########################################

//经过测试，cmd_si单步打印函数没有问题！！！！！！
static int cmd_si(char *args)
{
  //注意在这里char *args表示传入进来的继续往下执行多少步，但是记住会有缺省值为1的情况即传入进来的args可能为NULL
  if (args==NULL){args="1";}
  int N=atoi(args);
  cpu_exec(N);//调用cpu_exec函数继续执行N次
  return 0;//表示成功返回
}
//<--------打印寄存器和监视器信息实现（注意这里打印监视器还没有实现！！！）-------------->
static int cmd_info(char *args)
{
  //
  //printf("成功进入info函数\n");
  if (args==NULL && strcmp(args,"r")!=0 && strcmp(args,"w")!=0){return -1;}//考虑到用户可能故意刁难在info后面不写参数或者写了一大堆乱七八糟的参数，这里需要判断一下
  if (strcmp(args,"r")==0)
  {
    //如果参数输入的是r的话打印寄存器信息
    isa_reg_display();//调用nemu/src/isa/$ISA/reg.c中的接口void isa_reg_display(void)
  }
  return 0;
}

//<--------扫描内存函数实现-------------->
static int cmd_x(char *args)
{//先判断输入进来的表达式是否合法
  char *str_end = args + strlen(args);
  char *N = strtok(args, " ");
  if (N == NULL) { return -1; }
  char *EXPR = N + strlen(N) + 1;
    if (EXPR >= str_end) {
      EXPR = NULL;
      return -1;
    }
    printf("N的值为：%#x\n",atoi(N));
    uint32_t init_addr;
    sscanf(EXPR, "%x", &init_addr);
    printf("EXPR的值为：%#x\n",init_addr);
//然后根据EXPR的表达式结果往后寻找内存中的表达式
printf("\t虚拟内存地址\t虚拟内存地址对应的值\n");
for(int i=0;i<atoi(N);i++)
{
  uint32_t v_addr = init_addr + 4*i;
  uint32_t value = vaddr_read(v_addr,4);//往后读取4个字节
  printf("\t%#x\t%#x\n",v_addr,value);
}
return 0;
}//经过测试扫描函数实现没有问题！！！



static int cmd_p(char *args) {
  bool success=true;
	word_t res = expr(args,&success);
  if(!success){return -1;}
  printf("%d\n",res);
  return 0;
}
//#############################################################################################






static int cmd_help(char *args);
//这里是一个结构体数组，定义了三个结构体，结构体里面分别对应command的名称、描述、函数指针
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);//函数指针，它指向一个接受 char 指针参数并返回 int 值的函数。这个函数指针用于指向实际处理命令的函数
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si","Step Into N times which you input, default 1",cmd_si},
  {"info","Print Information According to Your Input",cmd_info},
  {"x","Solve the value of Expression, and print the following continuous N Bytes",cmd_x},
  {"p","Print the value of Expression",cmd_p}
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument 提取第一个参数*/
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given 如果什么参数都没有给*/
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {//调用rl_gets函数，读取一行的输入
    char *str_end = str + strlen(str);

    /* extract the first token as the command 提取第一个token出来*/
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments, 将剩下的字符串视为参数
     * which may need further parsing 可能需要进一步的解析
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif
    
    //把所有command命令全部查找一遍，寻找是否有对应位置的命令 
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0)//返回0表示两个命令相等
      {//调用cmd_table[i].handler函数指针并且传入参数args,如果处理函数返回负值，表示有错误或需要终止循环，函数返回。
      //比如输入了q，则调用cmd_q函数并且传入参数（这个参数可以为空、任何数都无所谓，反正用不上）
        if (cmd_table[i].handler(args) < 0) { return; }//注意在这里args是除了第一个参数，后面所有的字符串
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. 编译正则表达式*/
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
