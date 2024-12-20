#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS_LO16(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15)
#ifndef __riscv_e
#define REGS_HI16(f) \
                                    f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)
#define NR_REGS 32
#else
#define REGS_HI16(f)
#define NR_REGS 16
#endif

#define REGS(f) REGS_LO16(f) REGS_HI16(f)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE    ((NR_REGS + 3) * XLEN)
#define OFFSET_SP       ( 2 * XLEN)
#define OFFSET_CAUSE    ((NR_REGS + 0) * XLEN)
#define OFFSET_STATUS   ((NR_REGS + 1) * XLEN)
#define OFFSET_EPC      ((NR_REGS + 2) * XLEN)
#define OFFSET_SCRATCH  ((NR_REGS + 3) * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:
  #不过当你打算实现代码的时候, 你会发现在刚进入CTE而未切换到内核栈之前, 我们根本没有空闲的通用寄存器可以使用! 事实上, riscv32的设计非常精巧, 我们可以通过3条指令来完成内核栈的切换:
  csrrw sp, mscratch, sp   // (1) atomically exchange sp and mscratch 为了检查mscratch的值, 我们通过(1)巧妙地使用了CSR访问指令的原子交换功能来交换sp和mscratch的值, 这样既可以把原来的mscratch值读到通用寄存器sp中进行比较(riscv32的分支指令不能访问CSR寄存器), 又可以把原来的sp值保存到mscratch寄存器中避免被破坏. 
  bnez sp, save_context    // (2) take the branch if we trapped from user 接下来通过(2)判断刚才读出的mscratch值(即概念上的ksp): 如果值是0, 说明在进入CTE之前是内核态, 而此时内核栈的指针已经被(1)交换到mscratch寄存器中了,
  csrr sp, mscratch        // (3) if we trapped from kernel, restore the original sp 于是通过(3)把内核栈的指针读回来; 如果值不是0, 说明在进入CTE之前是用户态, 需要把sp切换到内核栈, 但指令(1)已经完成了这一切换工作了, 因此此时直接跳转到save_context处来保存上下文即可.

save_context:
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc
  csrr t3, mscratch
  
  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
  STORE t3, OFFSET_SCRATCH(sp)

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  #已经在__am_irq_handle函数的开头以及结尾分别调用了__am_get_cur_as以及__am_switch了这里就不用再多写了
  call __am_irq_handle   

  #修改CTE中__am_asm_trap()的实现, 使得从__am_irq_handle()返回后, 先将栈顶指针切换到新进程的上下文结构, 然后才恢复上下文, 从而完成上下文切换的本质操作
  mv sp, a0
  #将__am_irq_handle的返回值（存储在a0里面） 赋值给sp即可修改

  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  LOAD t3, OFFSET_SCRATCH(sp)
  csrw mstatus, t1
  csrw mepc, t2
  csrw mscratch, t3

  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE
  
  csrrw sp, mscratch, sp
  bnez sp, return_from_trap
  csrrw sp, mscratch, sp

return_from_trap:
  mret
