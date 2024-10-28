#include <am.h>  //来自于abstract-machine/am/include/am.h
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;


//cte表示context extension上下文扩展


Context* __am_irq_handle(Context *c) {
  // 可以在__am_irq_handle()中通过printf输出上下文c的内容, 然后通过简易调试器观察触发自陷时的寄存器状态, 从而检查你的Context实现是否正确
  if (user_handler) {
    Event ev = {0};
    // printf("mcause为:%d\n",c->mcause);
    switch (c->mcause) {
      case -1:ev.event= EVENT_YIELD;break;
      default: ev.event = EVENT_ERROR; break;//正是因为自己没有识别出自陷异常的操作，因此才会报错
    }

    c = user_handler(ev, c);
    
    // printf("\nc的内容为:\nmcause为:%u\nmstatus为:%u\n",c->mcause,c->mstatus);

    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

//另外两个统一的API:  bool cte_init(Context* (*handler)(Event ev, Context *ctx)) 以及 void yield()函数

// 用于进行CTE相关的初始化操作. 其中它还接受一个来自操作系统的事件处理回调函数的指针, 
// 当发生事件时, CTE将会把事件和相关的上下文作为参数, 来调用这个回调函数, 交由操作系统进行后续处理. 
bool cte_init(Context*(*handler)(Event, Context*)) {
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
  return NULL;
}


//用于进行自陷操作, 会触发一个编号为EVENT_YIELD事件. 不同的ISA会使用不同的自陷指令来触发自陷操作
void yield() {

#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");//这里手动加入一条ecall语句  将-1加载load到a7当中
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
