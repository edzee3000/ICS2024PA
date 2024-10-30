#include <common.h>

void init_mm(void);
void init_device(void);
void init_ramdisk(void);
void init_irq(void);
void init_fs(void);
void init_proc(void);

int main() {//简单梳理一下Nanos-lite目前的行为
  extern const char logo[];//打印Project-N的logo, 并通过Log()输出hello信息和编译时间
  printf("%s", logo);
  Log("'Hello World!' from Nanos-lite");   
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_mm();

  init_device();//调用init_device()对设备进行一些初始化操作. 目前init_device()会直接调用ioe_init().

  init_ramdisk();//初始化ramdisk. 一般来说, 程序应该存放在永久存储的介质中(比如磁盘). 但要在NEMU中对磁盘进行模拟是一个略显复杂工作, 因此先让Nanos-lite把其中的一段内存作为磁盘来使用. 这样的磁盘有一个专门的名字, 叫ramdisk.

#ifdef HAS_CTE
  init_irq();
#endif 

  init_fs();//init_fs()和init_proc(), 分别用于初始化文件系统和创建进程, 目前它们均未进行有意义的操作, 可以忽略它们.

  init_proc();

  Log("Finish initialization");

#ifdef HAS_CTE
  yield();
#endif

  panic("Should not reach here");//调用panic()结束Nanos-lite的运行.
}
