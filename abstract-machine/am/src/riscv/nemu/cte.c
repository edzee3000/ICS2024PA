#include <am.h>  //来自于abstract-machine/am/include/am.h
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;


//cte表示context extension上下文扩展


Context* __am_irq_handle(Context *c) {
  //在__am_irq_handle()的开头调用__am_get_cur_as() (在abstract-machine/am/src/$ISA/nemu/vme.c中定义), 来将当前的地址空间描述符指针保存到上下文中
  __am_get_cur_as(c);
  // 可以在__am_irq_handle()中通过printf输出上下文c的内容, 然后通过简易调试器观察触发自陷时的寄存器状态, 从而检查你的Context实现是否正确
  if (user_handler) {
    Event ev = {0};
    // printf("mcause为:%d\n",c->mcause);
    switch (c->mcause) {//根据c->mcause上下文信息判断对应的事件是什么  （即事件分发）
      case -1:  ev.event= EVENT_YIELD; break;  
      //观察navy-apps/libs/libos/src/syscall.h可得  当c->mcause为1~19时都属于syscall
      case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 10:
      case 11:case 12:case 13:case 14:case 15:case 16:case 17:case 18:case 19: 
      ev.event=EVENT_SYSCALL;break;
      default:  ev.event = EVENT_ERROR; break;//正是因为自己没有识别出自陷异常的操作，因此才会报错
    }



    //然后执行user_handler函数   即cte_init中传入的handler函数
    c = user_handler(ev, c);
    
    // printf("\nc的内容为:\nmcause为:%u\nmstatus为:%u\n",c->mcause,c->mstatus);

    assert(c != NULL);
  }

  assert(c->mepc >= 0x40000000 && c->mepc <= 0x88000000);
  // 在__am_irq_handle()返回前调用__am_switch() (在abstract-machine/am/src/$ISA/nemu/vme.c中定义)
  // 来切换地址空间, 将被调度进程的地址空间落实到MMU中
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

//另外两个统一的API:  bool cte_init(Context* (*handler)(Event ev, Context *ctx)) 以及 void yield()函数

// 用于进行CTE相关的初始化操作. 其中它还接受一个来自操作系统的事件处理回调函数的指针, 
// 当发生事件时, CTE将会把事件和相关的上下文作为参数, 来调用这个回调函数, 交由操作系统进行后续处理. 
bool cte_init(Context*(*handler)(Event, Context*)) {  //举个例子比如说main.c里面传了simple_trap函数作为CTE的h参数开始初始化作为这里的handler回调函数
  // 当我们选择yield test时, am-tests会通过cte_init()函数对CTE进行初始化, 其中包含一些简单的宏展开代码. 
  // 这最终会调用位于abstract-machine/am/src/$ISA/nemu/cte.c中的cte_init()函数.
  // cte_init()函数会做两件事情, 第一件就是设置异常入口地址:对于riscv32来说, 直接将异常入口地址设置到mtvec寄存器中即可.
  // cte_init()函数做的第二件事是注册一个事件处理回调函数, 这个回调函数由yield test提供

  // initialize exception entry  初始化异常入口
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler 注册一个事件处理回调函数
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *cp = (Context *)kstack.end - 1;//先强制类型转换一下然后再-1  所谓的-1  其实是从内存角度来看是将地址的值-sizeof(Context)  使得cp对应的context紧挨着kstack的end
  // cp->mepc = (uintptr_t)entry - 4;        //kcontext()要求内核线程不能从entry返回, 否则其行为是未定义的. 你需要在kstack的底部创建一个以entry为入口的上下文结构
  cp->mepc = (uintptr_t)entry;  

  //配合DiffTest   为了保证DiffTest的正确运行, 根据你选择的ISA, 你还需要进行一些额外的设置  但是这里可能有一点点问题
  cp->mstatus=0x1800;
  //注意这里跟手册上还是有一点点区别的  手册上面的cp直接是在ksatck的start那个位置的  但是这里我是直接在 (Context *)kstack.end - 1这个位置放了一个cp
  //注意我上面写的那一条注释是错误的！！！！！！！！！注意kcontext会将cp返回，然后将其地址的值赋值给pcb[0].cp也就是PCB的首地址存放的就是cp的地址，这恰恰好符合手册上面的图示
  //为此, 我们需要思考内核线程的调度会对分页机制造成什么样的影响. 内核线程和用户进程最大的不同, 
  // 就是它没有用户态的地址空间: 内核线程的代码, 数据和栈都是位于内核的地址空间. 那在启动分页机制之后, 
  // 如果__am_irq_handle()要返回一个内核线程的现场, 我们是否需要考虑通过__am_switch()切换到内核线程的虚拟地址空间呢?
  // 答案是, 不需要. 这是因为AM创建的所有虚拟地址空间都会包含内核映射, 无论在切换之前是位于哪一个虚拟地址空间,
  //  内核线程都可以在这个虚拟地址空间上正确运行. 因此我们只要在kcontext()中将上下文的地址空间描述符指针设置为NULL, 
  // 来进行特殊的标记, 等到将来在__am_irq_handle()中调用__am_switch()时, 如果发现地址空间描述符指针为NULL, 就不进行虚拟地址空间的切换.
  cp->pdir = NULL;


  cp->GPR2 = (uintptr_t)arg;//注意在这里根据abstract-machine/am/include/arch/riscv.h中的内容  #define GPR2 gpr[10]  GPR2为a0即gpr[10]  通过a0进行传参（因为只有一个void*参数  根据ABI约定）
  return cp;        
  //在 RISC-V 架构中，指令是 32 位的，所以减去 4 实际上是将地址回退到调用 entry 函数之前的指令。
  // 这样做的原因是，当异常处理完毕并从异常返回时，处理器应该继续执行 entry 函数调用之后的指令，而不是重新执行 entry 函数调用本身。

  //我们希望代码将来从__am_asm_trap()返回之后, 就会开始执行f(). 换句话说, 我们需要在kcontext()中构造一个上下文, 
  // 它指示了一个状态, 从这个状态开始, 可以正确地开始执行f(). 所以你需要思考的是, 为了可以正确地开始执行f(), 这个状态究竟需要满足什么样的条件?
  // 至于"先将栈顶指针切换到新进程的上下文结构", 很自然的问题就是, 新进程的上下文结构在哪里? 
  // 怎么找到它? 又应该怎么样把栈顶指针切换过去? 如果你发现代码跑飞了, 不要忘记, 程序是个状态机.
  /*
        |               |
        +---------------+ <---- kstack.end
        |               |
        |    context    |
        |               |
        +---------------+ <--+
        |               |    |
        |               |    |
        |               |    |
        |               |    |
        +---------------+    |
        |       cp      | ---+
        +---------------+ <---- kstack.start
        |               |
  */
}

//注意需要手动#define STACK_SIZE (4096 * 8)设置栈的大小为32KB
/*这个是在native当中实现的kcontext的部分  如果愿意的话可以进行参考
Context* kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *c = (Context*)kstack.end - 1;

  __am_get_example_uc(c);
  AM_REG_PC(&c->uc) = (uintptr_t)__am_kcontext_start;
  AM_REG_SP(&c->uc) = (uintptr_t)kstack.end;

  int ret = sigemptyset(&(c->uc.uc_sigmask)); // enable interrupt
  assert(ret == 0);

  c->vm_head = NULL;

  c->GPR1 = (uintptr_t)arg;
  c->GPR2 = (uintptr_t)entry;
  return c;
}

void yield() {
  raise(SIGUSR2);
}
*/

//用于进行自陷操作, 会触发一个编号为EVENT_YIELD事件. 不同的ISA会使用不同的自陷指令来触发自陷操作
//在GNU/Linux中, 用户程序通过自陷指令来触发系统调用, Nanos-lite也沿用这个约定. 
// CTE中的yield()也是通过自陷指令来实现, 虽然它们触发了不同的事件, 但从上下文保存到事件分发, 它们的过程都是非常相似的. 
void yield() {

#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");//这里手动加入一条ecall内联汇编语句  将-1加载load到a7当中
#endif
//asm是手动插入内联汇编语句



}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}




/*
AM在定义kcontext()的行为时, 还要求kcontext()只能在栈上放置一个上下文结构, 而不能放置更多的内容. 这样的要求有两点好处:

kcontext()对栈的写入只有一个上下文结构的内容, 而不会产生其它的副作用
OS可以预测调用kcontext()之后的返回值, 并且利用这一确定的特性进行一些检查或者简化某些实现
我们知道x86是通过栈来传递参数的, 如果kcontext()需要支持arg的传递, 它就需要往栈上放置更多的内容, 这样就违反了上述确定性了. 但在PA中, 这并不会导致致命的问题, 因此我们并不要求你的kcontext()实现严格遵守这一确定性. 但你还是可以思考如何在遵守确定性的情况下实现参数的传递.

一个解决方案是通过引入一个辅助函数来将真正的参数传递从kcontext()推迟到内核线程的运行时刻. 具体地, 我们可以在kcontext()中先把内核线程的入口设置为辅助函数, 并把参数设置到某个可用的寄存器中. 这样以后, 内核线程就会从辅助函数开始执行, 此时让辅助函数来把之前设置的参数从寄存器中放置到栈上, 再调用真正的线程入口函数(f()). 这一方案和Linux中加载用户程序还是有一些相似之处的: 用户程序在运行的时候也并不是直接把main()函数作为入口, 而是先从CRT定义的_start()开始运行, 进行包括设置参数在内的一系列初始化操作, 最后才调用main()函数.

如果你选择的ISA是x86, 你可以尝试在CTE中实现上述辅助函数. 考虑到要直接操作寄存器和栈, 这个辅助函数还是通过汇编代码来编写比较合适. 不过由于这个辅助函数的功能比较简单, 你只需要编写几条指令就可以实现它了.
*/