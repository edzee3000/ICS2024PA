.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mv s0, zero
  la sp, _stack_pointer
  call _trm_init

.size _start, . - _start


# 我们对编译得到的可执行文件的行为进行简单的梳理:
# 第一条指令从abstract-machine/am/src/$ISA/nemu/start.S开始, 设置好栈顶之后就跳转到abstract-machine/am/src/platform/nemu/trm.c的_trm_init()函数处执行.
# 在_trm_init()中调用main()函数执行程序的主体功能, main()函数还带一个参数, 目前我们暂时不会用到, 后面我们再介绍它.
# 从main()函数返回后, 调用halt()结束运行.




