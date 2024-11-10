#include <am.h>
#include <nemu.h>

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END); //Area heap结构用于指示堆区的起始和末尾
static const char mainargs[MAINARGS_MAX_LEN] = MAINARGS_PLACEHOLDER; // defined in CFLAGS
//用于输出一个字符
void putch(char ch) {
  outb(SERIAL_PORT, ch);
  // outb是在riscv.h当中定义的   
  // static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }//写入一个字节的数据
}
//用于结束程序的运行
void halt(int code) {
  nemu_trap(code);

  // should not reach here
  while (1);
}
//用于进行TRM相关的初始化工作
void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
