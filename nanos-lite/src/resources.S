.section .data
.global ramdisk_start, ramdisk_end
ramdisk_start:
.incbin "build/ramdisk.img"
ramdisk_end:

.section .rodata
.globl logo
logo:
.incbin "resources/logo.txt"
.byte 0






//make ARCH=$ISA-nemu
//会生成Nanos-lite的可执行文件, 编译期间会把ramdisk镜像文件nanos-lite/build/ramdisk.img 
//包含进Nanos-lite成为其中的一部分(在nanos-lite/src/resources.S中实现). 
//现在的ramdisk十分简单, 它只有一个文件, 就是我们将要加载的用户程序dummy, 
//这其实已经回答了上述第一个问题: 可执行文件位于ramdisk偏移为0处, 访问它就可以得到用户程序的第一个字节.