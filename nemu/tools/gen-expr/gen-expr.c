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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough 这么大的一个缓冲区应当是足够了的
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`比缓冲区要稍微大那么一丢丢
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


static char *buf_start = NULL;
static char *buf_end = buf+(sizeof(buf)/sizeof(buf[0]));

static int choose(int n) ;
void gen(char c) ;
void gen_num();
void gen_rand_op() ;

static void gen_rand_expr() {

  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
  
}



// 用于生成随机数字的函数
void gen_num() {
    int num = choose(100);//65596会数据溢出
  if (buf_start < buf_end) {
    int n_writes = snprintf(buf_start, buf_end-buf_start, "%d", num);
    if (n_writes > 0) {
      buf_start += n_writes;
    }
  }
}
// 用于生成括号的函数
void gen(char c) {
    // 打印字符，这里简单地使用 putchar 函数
    //putchar(c);
  int n_writes = snprintf(buf_start, buf_end - buf_start, "%c", c);
    if (buf_start < buf_end) { 
      if (n_writes > 0) {
        buf_start += n_writes;
      }
    }
}
// 用于生成随机操作符的函数
void gen_rand_op() {
    // 随机选择一个操作符
  char ops[] = {'+', '-', '*', '/'};
  char op = ops[choose(sizeof(ops))];
  gen(op);
}


static int choose(int n) {
    return rand() % n;
}






int main(int argc, char *argv[]) {
  int seed = time(0);//获取当前时间作为随机数生成的种子。
  srand(seed);//用获取的种子初始化随机数生成器。
  int loop = 1;//初始化一个变量 loop，用于控制循环次数。
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);//如果存在命令行参数，使用 sscanf 函数读取第一个参数（argv[1]），将其转换为整数，并赋值给 loop 变量。这允许用户指定循环的次数
  }
  int i;
  for (i = 0; i < loop; i ++) {//开始一个循环，循环次数由 loop 变量控制。
    buf_start=buf; gen_rand_expr(); *buf_start = '\0';

    sprintf(code_buf, code_format, buf);//使用 sprintf 函数将生成的随机表达式格式化到一个缓冲区 code_buf 中。

    FILE *fp = fopen("/tmp/.code.c", "w");//打开一个文件 /tmp/.code.c 用于写入，如果文件不存在则创建它。
    assert(fp != NULL);//使用 assert 宏检查文件是否成功打开，如果文件打开失败，则程序会终止。
    fputs(code_buf, fp);//将格式化后的代码写入到文件中
    fclose(fp);//关闭文件

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");//调用 system 函数，执行 gcc 命令来编译刚才写入的 C 代码文件，并生成可执行文件 /tmp/.expr。
    if (ret != 0) continue;//如果编译失败（ret 不为 0），则跳过当前循环的剩余部分。

    fp = popen("/tmp/.expr", "r");//使用 popen 函数执行编译后的程序，并打开一个管道来读取程序的输出。
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);//打印结果和生成的随机表达式
  }
  return 0;
}
//最后会发现大部分的表达式都会有  除0  和  数据溢出   的问题，  但是目前来说我还是没有思路去解决， 算了等以后再说吧
