#include <proc.h>

#include <declaration.h>

#define MAX_NR_PROC 4  //人为定义最大可以同时执行的进程为4个进程

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);//在nanos-lite/src/loader.c里面定义的函数

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  // int j = 1;
  static int j = 1;
  while (1) {
    // if(current==&pcb[0]) printf("目前执行的线程为:pcb[0]\n"); else if(current==&pcb[1]) printf("目前执行的线程为:pcb[1]\n");
    for(int i=0;i<MAX_NR_PROC;i++){if(current==&pcb[i])printf("目前执行的线程为:pcb[%d]",i); }
    
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg,  j);
    j ++;
    printf("j:%d\n",j);
    assert(0);
    yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, NULL);
  context_kload(&pcb[1], hello_fun, NULL);
  switch_boot_pcb();
  
  Log("Initializing processes...");
  

  
  // load program here 在这里加载程序
  // naive_uload(NULL,"/bin/dummy");
  // naive_uload(NULL,"/bin/hello");
  // naive_uload(NULL,"/bin/file-test");
  // naive_uload(NULL,"/bin/bmp-test");//会调用你实现的loader来加载第一个用户程序, 然后跳转到用户程序中执行. 如果你的实现正确, 你会看到执行dummy程序时在Nanos-lite中触发了一个未处理的4号事件. 这说明loader已经成功加载dummy, 并且成功地跳转到dummy中执行了. 关于未处理的事件, 我们会在下文进行说明.
  // naive_uload(NULL,"/bin/nslider");
  // naive_uload(NULL,"/bin/menu");
  // naive_uload(NULL,"/bin/nterm");
  // naive_uload(NULL,"/bin/bird");
  // naive_uload(NULL,"/bin/pal");
  // naive_uload(NULL,"/bin/typing-game");
  //  naive_uload(NULL,"/bin/dhrystone");
  // naive_uload(NULL,"/bin/fceux");
}

//Nanos-lite的schedule()函数
Context* schedule(Context *prev) {
  current->cp = prev;//保存上下文的指针  save the context pointer
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);//判断当前current是pcb[0]还是pcb[1]  如果是pcb[0]的话就切换为pcb[1]  switch between pcb[0] and pcb[1]
  //将当前的PCB的cp切换为先前的进程cp指针context pointer
  // printf("执行了schedule\n");  //初始化的时候注意*current = &pcb_boot因而是先进行线程切换  切换到pcb[0]了之后从回调函数schedule返回到trap.S当中的__am_asm_trap  然后执行下一个进程
  // assert(0);
  return current->cp; 
}


//Nanos-lite的context_kload()函数(框架代码未给出该函数的原型), 它进一步封装了创建内核上下文的过程: 
// 调用kcontext()来创建上下文, 并把返回的指针记录到PCB的cp中      
// 诶但是这里我就有一个问题了  如果这里是pcb->cp的话  那么在trap.S当中是不是就不需要我自己加的那一句 mv sp, a0了？
// 其实是我自己想错了， 因为__am_asm_trap根本就没有用到kcontext函数！！！  
void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  // Area stack;
  // stack.start = pcb->stack;
  // stack.end = pcb->stack + STACK_SIZE;
  pcb->cp = kcontext((Area){pcb->stack, pcb->stack+STACK_SIZE}, entry, arg);
  //仿照am-kernels/kernels/yield-os/yield-os.c的main函数里面的
  // pcb[0].cp = kcontext((Area) { pcb[0].stack, &pcb[0] + 1 }, f, (void *)1L);
  //这行代码去写，注意这里需要加一个STACK_SIZE因为后面有相关分页机制的实现！！！！！！而不是pcb + 1 ！！！！ 
}