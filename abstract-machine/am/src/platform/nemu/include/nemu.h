#ifndef NEMU_H__
#define NEMU_H__

#include <klib-macros.h>

#include ISA_H // the macro `ISA_H` is defined in CFLAGS
               // it will be expanded as "x86/x86.h", "mips/mips32.h", ...

#if defined(__ISA_X86__)
# define nemu_trap(code) asm volatile ("int3" : :"a"(code))
#elif defined(__ISA_MIPS32__)
# define nemu_trap(code) asm volatile ("move $v0, %0; sdbbp" : :"r"(code))
#elif defined(__riscv)
# define nemu_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))
#elif defined(__ISA_LOONGARCH32R__)
# define nemu_trap(code) asm volatile("move $a0, %0; break 0" : :"r"(code))
#else
# error unsupported ISA __ISA__
#endif

#if defined(__ARCH_X86_NEMU)
# define DEVICE_BASE 0x0
#else
# define DEVICE_BASE 0xa0000000
#endif


// 在nemu.h当中定义了内存映射输入输出的基址为0xa0000000
// 另外由于在PA实验中我使用的是riscv32架构的ISA指令集因此device基址为0xa0000000
// 之后所有的device设备都是从0xa0000000这里开始往后算基址
// ####################################################################################################
// ################# 注意：nemu.h相当重要（个人认为），它是沟通了上层am与下层nemu之间的桥梁 ######################
/*
    我之前一直想不明白，既然上层软件am与下层硬件nemu实现了解耦之后，那么用什么给它们联系在一起呢？
    我找了半天也没有找到在am里面使用nemu的任何一条语句，也没有在nemu里面找到am的任何一条语句，百思不得其解
    后来突然恍然大悟，确实am与nemu既然是解耦的，那么它们之间的“桥梁”只是一系列的约定即可，也就是在am当中platform里面需要的nemu.h
    比如对于一个typing_game的程序，在上层抽象机里面需要调用io_write函数，然后去调用ioe_write函数，然后再转到对应device设备的
    编译器将整个typing_game程序在am当中编译链接之后形成hello-riscv32-nemu.bin、elf、txt文件，
    然后将它们传给nemu/build/riscv32-nemu-interpreter硬件去执行，然后nemu作为底层硬件去根据指令一条一条去执行，
    并且由于device和isa是没有关系的，因此只要规定好设备的接口，比如SERIAL_PORT、KBD_ADDR、RTC_ADDR等设备地址
    这样的话底层硬件nemu执行命令的时候就不用管自己是不是在访问设备（其实我们自己写代码的时候还是要去判断是在访问设备还是在访问物理内存），
    而是直接调用paddr.c中的paddr_read()函数（里面我们已经写好了pmem_read和mmio_read两种访问）  
    这样就可以实现访问设备device了，与ISA是什么根本没有任何关系，而且这些接口（姑且叫它们是接口吧）规定是需要我们自己去规定的
    就比如下面这些define以及在menuconfig里面规定的那些，只要am和nemu共同遵守这些约定就行
 */
// ####################################################################################################


#define MMIO_BASE 0xa0000000
 
#define SERIAL_PORT     (DEVICE_BASE + 0x00003f8)  //串口端口
#define KBD_ADDR        (DEVICE_BASE + 0x0000060)  //设备的键盘地址
#define RTC_ADDR        (DEVICE_BASE + 0x0000048)  //RTC实时时钟地址
#define VGACTL_ADDR     (DEVICE_BASE + 0x0000100)  //VDACTL的地址
#define AUDIO_ADDR      (DEVICE_BASE + 0x0000200)
#define DISK_ADDR       (DEVICE_BASE + 0x0000300)
#define FB_ADDR         (MMIO_BASE   + 0x1000000)  //Flame Buffer帧缓冲区的物理地址
#define AUDIO_SBUF_ADDR (MMIO_BASE   + 0x1200000)

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)
#define NEMU_PADDR_SPACE \
  RANGE(&_pmem_start, PMEM_END), \
  RANGE(FB_ADDR, FB_ADDR + 0x200000), \
  RANGE(MMIO_BASE, MMIO_BASE + 0x1000) /* serial, rtc, screen, keyboard */  //NEMU_PADDR_SPACE物理地址空间

typedef uintptr_t PTE;

#define PGSIZE    4096

#endif
