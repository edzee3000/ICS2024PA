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

  init_device();

  init_ramdisk();

#ifdef HAS_CTE
  init_irq();
#endif

  init_fs();

  init_proc();

  Log("Finish initialization");

#ifdef HAS_CTE
  yield();
#endif

  panic("Should not reach here");
}
