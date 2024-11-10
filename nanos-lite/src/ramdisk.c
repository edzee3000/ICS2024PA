#include <common.h>

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

/* The kernel is monolithic, therefore we do not need to  
 * translate the address `buf' from the user process to
 * a physical one, which is necessary for a microkernel.
 * 内核是单体的，因此我们不需要将用户进程中的地址 buf 转换为物理地址，这对于微内核来说是必要的。
 */

//由于ELF文件在ramdisk中, 框架代码提供了一些ramdisk相关的函数(在nanos-lite/src/ramdisk.c中定义), 你可以使用它们来实现loader的功能:

//事实上, loader的工作向我们展现出了程序的最为原始的状态: 比特串! 
// 加载程序其实就是把这一毫不起眼的比特串放置在正确的位置, 但这其中又折射出"存储程序"的划时代思想: 
// 当操作系统将控制权交给它的时候, 计算机把它解释成指令并逐条执行. 
// loader让计算机的生命周期突破程序的边界: 一个程序结束并不意味着计算机停止工作, 计算机将终其一生履行执行程序的使命.

/* read `len' bytes starting from `offset' of ramdisk into `buf'
  从 offset 开始，读取 len 个字节的 ramdisk 数据到 buf 中 */
size_t ramdisk_read(void *buf, size_t offset, size_t len) {
  // printf("%u",RAMDISK_SIZE);
  assert(offset + len <= RAMDISK_SIZE);
  memcpy(buf, &ramdisk_start + offset, len);  //我一直不理解为什么这里要用到&取地址符？？？？##################################
  return len;
}

/* write `len' bytes starting from `buf' into the `offset' of ramdisk
  将 len 个字节的数据从 buf 写入到 ramdisk 的 offset 位置。 */
size_t ramdisk_write(const void *buf, size_t offset, size_t len) {
  assert(offset + len <= RAMDISK_SIZE);
  memcpy(&ramdisk_start + offset, buf, len);
  return len;
}

void init_ramdisk() {
  Log("ramdisk info: start = %p, end = %p, size = 0x%x bytes",
      &ramdisk_start, &ramdisk_end, RAMDISK_SIZE);
}

size_t get_ramdisk_size() {
  return RAMDISK_SIZE;
}
