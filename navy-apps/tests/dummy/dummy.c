#include <stdint.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);


// 我们要在Nanos-lite上运行的第一个用户程序是navy-apps/tests/dummy/dummy.c. 
// 为了避免和Nanos-lite的内容产生冲突, 我们约定目前用户程序需要被链接到内存位置0x3000000(x86)或0x83000000(mips32或riscv32)附近, 
// Navy已经设置好了相应的选项(见navy-apps/scripts/$ISA.mk中的LDFLAGS变量)

int main() {
  return _syscall_(SYS_yield, 0, 0, 0);
}
