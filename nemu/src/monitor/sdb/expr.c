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

/* We use the POSIX regex functions to process regular expressions.使用POSIX正则函数处理正则表达式
 * Type 'man regex' for more information about POSIX regex functions.按下man regex寻找更多关于POSIX正则函数内容
 */
#include <regex.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../isa/riscv32/local-include/reg.h"

extern const char *regs[];
// const char *regs[] = {
//   "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
//   "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
//   "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
//   "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
// };

enum {
  TK_NOTYPE = 256, TK_EQ=0,NOT_EQ,
  ADD,SUB,MUL,DIV,LEFT_PAR,RIGHT_PAR,
  /* TODO: Add more token types添加更多的token种类 */
  DECIMAL_NUM,HEX_NUM,REGISTER,
  NOT,AND,OR,
  DEREF,TK_NEG
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.添加更多的规则
   * Pay attention to the precedence level of different rules.注意不同规则之间的优先级关系
   */

  {" +", TK_NOTYPE},    // spaces（一个或多个空格）
  {"\\+", ADD},         // plus
  {"\\-", SUB},         //subtract
  {"==", TK_EQ},        // equal
  {"!=", NOT_EQ}, //NOT_EQUAL   
  {"\\/", DIV},         // divide
  {"\\*",MUL},//Multiply
  {"\\(",LEFT_PAR},//左括号
  {"\\)",RIGHT_PAR},//右括号
  {"0[xX][0-9a-fA-F]+",HEX_NUM},//输入一个16进制数
  {"[0-9]+", DECIMAL_NUM}, //有关于输入一大堆十进制数字的字符串
  {"\\|\\|",OR},//或
  {"&&",AND},//与
  {"!",NOT},//非
  {"\\$[\\$a-z0-9]+",REGISTER}//寄存器名字（呃呃做了一半发现其实并没有用到……结果做完了发现题目好像又需要这个……唉……）
  //先不急着实现那么复杂的表达式，先把最基本的加减乘除的运算弄好再说
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.  规则需要被使用很多次
 * Therefore we compile them only once before any usage.  因此我们只需要在他们使用之前编译他们一次即可
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);//regcomp编译正则表达式
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  //我们使用Token结构体来记录token的信息
  int type;//type成员用于记录token的类型. 大部分token只要记录类型就可以了, 例如+, -, *, /
  char str[32];//如果我们只记录了一个十进制整数token的类型, 在进行求值的时候我们还是不知道这个十进制整数是多少. 这时我们应该将token相应的子串也记录下来, str成员就是用来做这件事情的.
  //需要注意的是, str成员的长度是有限的, 当你发现缓冲区将要溢出的时候, 要进行相应的处理(思考一下, 你会如何进行处理?), 否则将会造成难以理解的bug.
} Token;

static Token tokens[128] __attribute__((used)) = {};//tokens数组用于按顺序存放已经被识别出的token信息//我偷偷把32改成128应该不过分吧……
static int nr_token __attribute__((used))  = 0;//nr_token指示已经被识别出的token数目


bool check_parentheses(int p ,int q);//函数声明
int dominant_operator(int p , int q);
int priority(int token_type);
int eval(int p,int q);
void judge_DEREF(int i);
void judge_NEG(int i);
word_t vaddr_read(vaddr_t addr, int len) ;

static bool make_token(char *e) {
  //用position变量来指示当前处理到的位置, 并且按顺序尝试用不同的规则来匹配当前位置的字符串. 
  //当一条规则匹配成功, 并且匹配出的子串正好是position所在位置的时候, 我们就成功地识别出一个token, 
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;//表明已经识别出多少个token了

  while (e[position] != '\0') {
    /* Try all rules one by one. 一个一个尝试规则*/
    for (i = 0; i < NR_REGEX; i ++) {
      //一个规则一个规则尝试如果匹配成功则进入if并且break不用循环了，如果没有成功则继续for循环直到最后判断都没有匹配
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {//regexec 匹配我们的目标文本串
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        //Log()宏会输出识别成功的信息. 你需要做的是将识别出的token信息记录下来(一个例外是空格串),
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes 现在一个新的token被识别为rule[i]，添加代码记录在tokes数组下的token
         * to record the token in the array `tokens'. For certain types 对于特定的token种类，需要一些其他的行动
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE:nr_token--;break;//TK_NOTYPE的情况可以直接丢弃掉
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case LEFT_PAR:
        case RIGHT_PAR:
        case TK_EQ:
        case NOT_EQ:
        case NOT:
        case AND:
        case OR: tokens[nr_token].type=rules[i].token_type;break;

        case DECIMAL_NUM:
          tokens[nr_token].type=DECIMAL_NUM;
          strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          //<----------------------这里需要小心万一str长度太大了可能会超出数组原本大小--------------------------->
          break;
        case HEX_NUM:
          tokens[nr_token].type=HEX_NUM;
          strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          //<------------------注意这里是直接将16进制数以字符串形式输入进去，并没有进行相应的转换！！！---------------------->
          break;
        case REGISTER:
          tokens[nr_token].type=REGISTER;
          int num_regs=   32;//sizeof(regs) / sizeof(regs[0]);//注意这里会报错错误使用sizeof，好像是因为regs是外部引用的，不知道它数组大小是多少
          char name[32];
          strncpy(name, &e[position-substr_len+1],substr_len-1);
          name[substr_len-1]='\0';
          int idx;
          for(idx=0;idx<num_regs;idx++){
            if(strcmp(reg_name(idx),name)==0)
            {
              u_int32_t value_reg=gpr(idx);//注意这里我存的内容不是名字而是寄存器对应的值
              sprintf(tokens[nr_token].str,"%x",value_reg);
              break;
              } 
          }

          // if(idx==num_regs)assert(0);//表示输入寄存器名字有问题
          if(idx==num_regs){printf("输入的寄存器名字有问题\n");return false;}
          break;
        default: assert(0);
        }
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. *///插入计算表达式的代码
  int i=0;
  for(i=0;i<nr_token;i++)
  {
  judge_DEREF(i);
  judge_NEG(i);
  }



  for(i=0;i<nr_token;i++)//打印类型内容//之后是需要注释掉的的
  {
    if(tokens[i].type==DECIMAL_NUM){printf("toke%d类型为:%d ,内容为：%d\n",i,tokens[i].type, atoi(tokens[i].str));}
    else if(tokens[i].type==HEX_NUM||tokens[i].type==REGISTER){ u_int32_t value;
      sscanf(tokens[i].str,"%x",&value);
      printf("toke%d类型为:%d ,内容为：%#x\n",i,tokens[i].type,value);}
    else{printf("toke%d类型为:%d\n",i,(char)tokens[i].type);}
  }




  //准备工作做完之后开始计算表达式的值
  u_int32_t res= eval(0,nr_token-1);

  return res;
}
// p (1 2 3   +4)  for test


void judge_DEREF(int i)
{
  if (tokens[i].type == MUL && 
  (i == 0 || 
  (tokens[i - 1].type <=DIV &&tokens[i - 1].type >=ADD) || tokens[i - 1].type ==TK_NEG ||
   (tokens[i - 1].type == NOT || tokens[i - 1].type ==AND ||tokens[i - 1].type ==OR) ||
   tokens[i - 1].type == NOT_EQ || tokens[i - 1].type ==TK_EQ || tokens[i - 1].type ==LEFT_PAR) ) 
    tokens[i].type = DEREF;
}

void judge_NEG(int i)
{
  if(tokens[i].type== SUB&&
  (i==0||
  (tokens[i-1].type!=DECIMAL_NUM&&
  tokens[i-1].type!=HEX_NUM)||
   tokens[i-1].type==LEFT_PAR))
        tokens[i].type = TK_NEG;
}

//括号匹配函数
bool check_parentheses(int p ,int q){
   // printf("--------------\n");  
    int i,tag = 0;
    if(tokens[p].type != LEFT_PAR || tokens[q].type != RIGHT_PAR) return false; //首尾没有()则为false 
    for(i = p ; i <= q ; i ++){    
        if(tokens[i].type == LEFT_PAR) tag++;
        else if(tokens[i].type == RIGHT_PAR) tag--;
        if(tag == 0 && i < q) return false ;  //(3+4)*(5+3) 返回false
    }                              
    if( tag != 0 ) return false;   
    return true;                   
} 


//寻找主操作符函数
int dominant_operator(int p , int q){         
  int i ,dom = p, left_n = 0;
  int pr = -1 ;
  for(i = p ; i <= q ; i++){
      if(tokens[i].type == LEFT_PAR){
          left_n ++;
          i++;
          while(1){
              if(tokens[i].type == LEFT_PAR) left_n ++;
              else if(tokens[i].type == RIGHT_PAR) left_n --;
              i++;
              if(left_n == 0)
                  {i--;break;}//注意由于上面i++了因此在这里需要对i进行--操作
          }  
          if(i > q)break;
      }      
      else if(tokens[i].type == DECIMAL_NUM) continue;
      else if(priority(tokens[i].type) >= pr){//主操作运算符也是要按照顺序来，比如说
          pr = priority(tokens[i].type);
          dom = i;
      }      
  }          
  // printf("%d\n",left_n);
  // printf("主操作符位置为：%d\n",dom);
  // printf("主操作符类型为：%d\n",tokens[dom].type);
  return dom;//注意返回的dom是位置   tokens[dom]才是主运算符对应的token
}             
//判断优先级数值函数
int priority(int token_type)
{
  if (token_type==TK_NEG)return 1;
  if (token_type==REGISTER)return 2;
  if (token_type==NOT||token_type==DEREF)return 3;
  if (token_type==DIV||token_type==MUL)return 4;
  if (token_type==ADD||token_type==SUB)return 5;
  if (token_type==TK_EQ||token_type==NOT_EQ)return 6;
  if (token_type==AND)return 7;
  if (token_type==OR)return 8;
  
  
  return -1;
}

//计算从p开始到q之间表达式的值
int eval(int p,int q) {
  if (p > q) {
    /* Bad expression */
    assert(0);//表明这个表达式有问题
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    u_int32_t value;
    switch (tokens[p].type){
    case DECIMAL_NUM:
      sscanf(tokens[p].str,"%d",&value);
      printf("十进制数为：%u\n",value);
      return value;break;
    case HEX_NUM: 
      sscanf(tokens[p].str,"%x",&value);
      // printf("十六进制数为：%u\n",value);
      return value;break;
    case REGISTER: 
      sscanf(tokens[p].str,"%x",&value);
      // printf("十六进制数为：%#x\n",value);
      return value;break;
    default:assert(0);
      break;
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    int op = dominant_operator(p,q);
    printf("op值为：%d\n",op);
    if (tokens[op].type==TK_NEG){return -eval(op+1,q);}
    else if (tokens[op].type==DEREF){return vaddr_read(eval(op+1,q),4);}
    else if (tokens[op].type==NOT){return !eval(op+1,q);}

    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case ADD: return val1 + val2;
      case SUB: return val1 - val2;
      case MUL: return val1 * val2;
      case DIV:
      if (val2==0){printf("发生表达式除以0的错误，程序退出");assert(0);} 
      return val1 / val2;
      case TK_EQ:return (val1==val2);
      case NOT_EQ:return (val1!=val2);
      case AND:return (val1 && val2);
      case OR:return (val1 || val2);
      // case DEREF: vaddr_read();
      default: assert(0);
    }
  }
  assert(0);
}

//完了做了一半发现题目要求全都是进行无符号运算，啊啊啊啊啊啊啊啊得重新修改了