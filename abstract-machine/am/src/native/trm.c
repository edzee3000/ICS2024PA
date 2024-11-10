#include <am.h>
#include <stdio.h>
#include <klib-macros.h>

void __am_platform_dummy();
void __am_exit_platform(int code);

//用于进行TRM相关的初始化工作
void trm_init() {
  __am_platform_dummy();
}
//用于输出一个字符   这里的putch相当于一层接口 隔绝了直接调用底层的代码
void putch(char ch) {
  putchar(ch); //putchar函数就是输出单个字符
}
//用于结束程序的运行
void halt(int code) {
  const char *fmt = "Exit code = 40h\n";
  for (const char *p = fmt; *p; p++) {
    char ch = *p;
    if (ch == '0' || ch == '4') {
      ch = "0123456789abcdef"[(code >> (ch - '0')) & 0xf];//我的妈呀这个是啥骚操作  算了以后再回来看吧
    }//回来补坑了  这里的if里面的操作  是将code的低8位输出出来  比如code=0x12345678  那么结果就是输出78  可以啊又学会了截取位串的新方式了
    putch(ch);
  }
  __am_exit_platform(code);
  putstr("Should not reach here!\n");
  while (1);
}

Area heap = {};
