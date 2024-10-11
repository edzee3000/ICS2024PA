#ifndef RISCV_H__
#define RISCV_H__

#include <stdint.h>

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }//读取一个字节的数据
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }//读取一个字的数据
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }//读取一个字长的数据

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }//写入一个字节的数据
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }//写入一个字的数据
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }//写入一个字长的数据

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80
//PTE_V、PTE_R、PTE_W、PTE_X、PTE_U、PTE_A、PTE_D 是与页表条目（Page Table Entry）相关的宏定义，
//它们代表了页表条目的不同属性。例如，PTE_V 表示页表条目是有效的，PTE_R、PTE_W、PTE_X 分别表示页表条目的读、写、执行权限。

enum { MODE_U, MODE_S, MODE_M = 3 };//枚举类型  代表了 RISCV 的不同特权模式。MODE_U 是用户模式，MODE_S 是超级用户模式，MODE_M 是机器模式。
#define MSTATUS_MXR  (1 << 19)
#define MSTATUS_SUM  (1 << 18)


//#if __riscv_xlen == 64 检查 RISC-V 架构的字长（xlen）是否为 64 位。
//如果是，MSTATUS_SXL 和 MSTATUS_UXL 将被定义为特定的值，否则它们将被定义为 0。
//这些宏用于设置或清除 MSTATUS 寄存器中的 SXL 和 UXL 位，这些位与处理器的扩展状态有关。
#if __riscv_xlen == 64
#define MSTATUS_SXL  (2ull << 34)
#define MSTATUS_UXL  (2ull << 32)
#else
#define MSTATUS_SXL  0
#define MSTATUS_UXL  0
#endif

#endif
