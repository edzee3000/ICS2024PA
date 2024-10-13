#include <common.h>
#include <isa.h>
#include <elf.h>
#include <iringbuf.h>
#include <memory/paddr.h>
#include <utils.h>

//##################################由于不太会写头文件因此在cpu-exec.c当中实现iringbuf#####################################
//##################################打算在这里实现输出iringbuf的功能######################################################
//打算在这里去实现iringbuf指令环形缓冲区的功能  先定义一个iringbuf缓冲区节点

int num_node=0;
IringNode* head;
IringNode* tail;

//定义更新buf的函数
void Update_Buf(word_t pc,uint32_t instr)
{
  IringNode *new_node=(IringNode *)malloc(sizeof(IringNode));
  new_node->pc=pc;  new_node->instr=instr;   new_node->next=NULL;
  //针对是否到达16个节点的buf进行分类讨论
  if(num_node==0) { head=tail=new_node;num_node++;  return;}
  if(num_node<MAX_NUM_NODE)
  { tail->next = new_node;
    tail=tail->next;
    num_node++;return;}
  else{tail->next=new_node;
    new_node->next=NULL;tail=tail->next;
    IringNode *temp=head; head=head->next; free(temp);return;}//这里差点就内存泄露了……
}
//定义打印buf的函数
void display_iringbuf()
{
  char buf[1024]; // 1024应该足够了吧……
  char *p;
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  printf("最新执行的几条指令有：\n");
  IringNode *temp=head;
  while(temp!=NULL)
  {
    p=buf;
    p += sprintf(buf, "%s" FMT_WORD ": %08x ", temp->next==NULL ? " --> ":"     ", temp->pc, temp->instr);
    disassemble(p, buf+sizeof(buf)-p,  temp->pc, (uint8_t *)&(temp->instr), 4);
    if(temp->next==NULL) printf(ANSI_FG_RED);
    puts(buf);
    temp=temp->next;
  }
  puts(ANSI_NONE);
}
//##########################################################################
