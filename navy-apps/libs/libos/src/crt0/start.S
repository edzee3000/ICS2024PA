.globl  _start
_start:

// 用户程序的入口位于navy-apps/libs/libos/src/crt0/start.S中的_start()函数,
// 这里的crt是C RunTime的缩写, 0的含义表示最开始. 
// _start()函数会调用navy-apps/libs/libos/src/crt0/crt0.c中的call_main()函数, 
// 然后调用用户程序的main()函数, 从main()函数返回后会调用exit()结束运行.

#if defined(__ISA_AM_NATIVE__)

  movq $0, %rbp
  // (rsp + 8) should be multiple of 16 when
  // control is transfered to the function entry point.
  // See amd64 ABI manual for more details
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
  mv s0, zero
  jal call_main

#elif defined(__ISA_LOONGARCH32R__)

  move $fp, $zero
  bl call_main

#else
#error Unsupported ISA
#endif
