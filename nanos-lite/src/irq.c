#include <common.h>


void do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
  //Nanos-lite的事件处理回调函数默认不处理所有事件, 你需要在其中识别出自陷事件EVENT_YIELD, 然后输出一句话即可, 目前无需进行其它操作
  //重新运行Nanos-lite, 如果你的实现正确, 你会看到识别到自陷事件之后输出的信息, 并且最后仍然触发了main()函数末尾设置的panic().
  
  // printf("上下文c当中的cause原因为:%d\n",c->mcause);
  switch (e.event)
  {case EVENT_YIELD:  printf("识别到EVENT_YIELD自陷事件，编号为1\n");  break;
  case EVENT_SYSCALL: printf("识别到EVENT_SYSCALL系统调用事件，编号为2\n");  break;
  default:break;}
 

  switch (e.event) {
    case EVENT_YIELD: printf("识别到自陷事件\n");    break;
    case EVENT_SYSCALL: do_syscall(c);  break;     //表示接收到了一个 系统调用syscall的请求  然后根据c里面的mcause再去分别处理  这里其实是做了一层抽象  将事件event和上下文c分离开来 
    default: panic("Unhandled event ID = %d", e.event);
  }
  // 返回输入的上下文 c，表示处理事件后的上下文状态。  
  // 注意在执行do_syscall的时候传入的是Context指针c，因此在里面可能会对c进行更改，
  // 比如对于c的gpr[NR_REGS], mcause, mstatus, mepc等等进行修改  （？？？？？？？？？？？？？？？会不会修改嘞？？？）
  return c;
}
//irq全称为Interrupt Request中断请求  当一个硬件设备需要CPU的服务时，它会发送一个中断信号，这个信号就是中断请求。
void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);//在这里do_event函数作为上下文扩展的回调函数
  // 这里相当于am-kernels/tests/am-tests/include/amtest.h当中yield test里的 
  // #define CTE(h) ({ Context *h(Event, Context *); cte_init(h); }) 
  // 然后传进去CTE(simple_trap)作为回调函数
}
