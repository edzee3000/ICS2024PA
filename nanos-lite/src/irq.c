#include <common.h>

static Context* do_event(Event e, Context* c) {
  //Nanos-lite的事件处理回调函数默认不处理所有事件, 你需要在其中识别出自陷事件EVENT_YIELD, 然后输出一句话即可, 目前无需进行其它操作
  //重新运行Nanos-lite, 如果你的实现正确, 你会看到识别到自陷事件之后输出的信息, 并且最后仍然触发了main()函数末尾设置的panic().
  
  // printf("自陷事件的编号为:%d\n",e.event);

  switch (e.event) {
    case EVENT_YIELD:printf("识别到自陷事件\n");    break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
