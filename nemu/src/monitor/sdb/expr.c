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


enum {
  TK_NOTYPE = 256, TK_EQ=0,
  ADD,SUB,MUL,DIV,LEFT_PAR,RIGHT_PAR,
  /* TODO: Add more token types添加更多的token种类 */
  DECIMAL_NUM
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
  {"\\/", DIV},         // divide
  {"\\*",MUL},//Multiply
  {"\\(",LEFT_PAR},
  {"\\)",RIGHT_PAR},
  {"[0-9]+", DECIMAL_NUM}, //有关于输入一大堆十进制数字的字符串

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

static Token tokens[32] __attribute__((used)) = {};//tokens数组用于按顺序存放已经被识别出的token信息
static int nr_token __attribute__((used))  = 0;//nr_token指示已经被识别出的token数目


bool check_parentheses(int p ,int q);//函数声明
int dominant_operator(int p , int q);
int priority(int token_type);
int eval(int p,int q);

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
          tokens[nr_token].type=ADD;
          // strcpy(tokens[nr_token].str,"\0");
          break;
        case SUB:
          tokens[nr_token].type=SUB;
          break;
        case MUL:
          tokens[nr_token].type=MUL;
          break;
        case DIV:
          tokens[nr_token].type=DIV;
          break;
        case LEFT_PAR:
          tokens[nr_token].type=LEFT_PAR;
          break;
        case RIGHT_PAR:
          tokens[nr_token].type=RIGHT_PAR;
          break;
        case DECIMAL_NUM:
          tokens[nr_token].type=DECIMAL_NUM;
          strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          //<----------------------这里需要小心万一str长度太大了可能会超出数组原本大小--------------------------->
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
    if(tokens[i].type==DECIMAL_NUM){printf("toke%d类型为:%d ,内容为：%d\n",i,tokens[i].type, atoi(tokens[i].str));}
    else{printf("toke%d类型为:%d\n",i,(char)tokens[i].type);}
  }
  //开始计算表达式的值
  int res= eval(0,nr_token-1);

  return res;
}
// p (1 2 3   +4)  for test


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
          left_n += 1;
          i++;
          while(1){
              if(tokens[i].type == LEFT_PAR) left_n += 1;
              else if(tokens[i].type == RIGHT_PAR) left_n --;
              i++;
              if(left_n == 0)
                  break;
          }  
          if(i > q)break;
      }      
      else if(tokens[i].type == DECIMAL_NUM) continue;
      else if(priority(tokens[i].type) > pr){
          pr = priority(tokens[i].type);
          dom = i;
      }      
  }          
  // printf("%d\n",left_n);
  return dom;//注意返回的dom是位置   tokens[dom]才是主运算符对应的token
}             
//判断优先级数值函数
int priority(int token_type)
{
  if (token_type==ADD||token_type==SUB)return 1;
  if (token_type==DIV||token_type==MUL)return 2;
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
    return atoi(tokens[p].str);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    int op = dominant_operator(p,q);
    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case ADD: return val1 + val2;
      case SUB: return val1 - val2;
      case MUL: return val1 * val2;
      case DIV: return val1 / val2;
      default: assert(0);
    }
  }
  assert(0);
}