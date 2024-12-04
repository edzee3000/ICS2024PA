.globl  _start
_start:


// 用户程序的入口位于navy-apps/libs/libos/src/crt0/start.S中的_start()函数,
// 这里的crt是C RunTime的缩写, 0的含义表示最开始. 
// _start()函数会调用navy-apps/libs/libos/src/crt0/crt0.c中的call_main()函数, 
// 然后调用用户程序的main()函数, 从main()函数返回后会调用exit()结束运行.

#if defined(__ISA_AM_NATIVE__)

  // 初始化基指针（rbp）为0
  movq $0, %rbp
  // (rsp + 8) should be multiple of 16 when
  // control is transfered to the function entry point.
  // See amd64 ABI manual for more details
  // 为了让AM native正确运行, 你也需要在Navy的_start中设置正确的栈指针.
  // 假设%rax寄存器已经包含了栈顶地址
  // 将%rax的值赋给栈指针%rsp
  movq %rax, %rsp
  // 确保栈指针16字节对齐
  // 在AMD64 ABI中，函数调用前rsp需要16字节对齐
  andq $0xfffffffffffffff0, %rsp
  movq %rax, %rdi
  call call_main

#elif defined(__ISA_X86_64__)

  movq $0, %rbp
  movq %rax, %rsp
  movq %rax, %rdi
  call call_main

#elif defined(__ISA_X86__)

  movl $0, %ebp
  call call_main

#elif defined(__ISA_MIPS32__)
  
  move $fp, $zero
  jal call_main

#elif defined(__riscv)
  // 在navy的start.S当中设置正确的栈指针  由于Nanos-lite和Navy作了一项约定把栈顶位置设置到GPRx即a0当中，因而将a0赋值给sp即可
  mv sp, a0

  mv s0, zero
  jal call_main

#elif defined(__ISA_LOONGARCH32R__)

  move $fp, $zero
  bl call_main

#else
#error Unsupported ISA
#endif





//
//哎呀, 栈指针寄存器可是ISA相关的, 在Nanos-lite里面不方便处理. 别着急, 还记得用户进程的那个_start吗? 在那里可以进行一些ISA相关的操作. 
//于是Nanos-lite和Navy作了一项约定: Nanos-lite把栈顶位置设置到GPRx中, 然后由Navy里面的_start来把栈顶位置真正设置到栈指针寄存器中.