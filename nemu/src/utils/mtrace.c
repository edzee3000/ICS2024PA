#include <common.h>
#include <isa.h>
#include <elf.h>
#include <mtrace.h>
#include <memory/paddr.h>
#include <utils.h>



//#########################由于不是非常会写头文件，因而将mtrace功能实现在vadrr.c中#############################
//#################定义一个MTRACE_NODE的作为记录MTRACE节点##############
#ifdef CONFIG_MTRACE
MTRACE_NODE *head_mtrace=NULL;
MTRACE_NODE *tail_mtrace=NULL;
//##################################################################
//下面都是MTRACE相关函数实现
void trace_memory(vaddr_t addr,word_t content)
{
  MTRACE_NODE *node=(MTRACE_NODE *)malloc(sizeof(MTRACE_NODE));  node->addr=addr;  node->content=content; node->next=NULL;
  if(head_mtrace==NULL) {head_mtrace=node;tail_mtrace=node;}
  else{tail_mtrace->next=node; tail_mtrace=node; }
}
void print_trace_memory()
{
  MTRACE_NODE *node=head_mtrace;
  printf("历史访存数据为：\n");
  while(node!=NULL)
  {
    printf("\t0x%#x: 0x%#x \n",node->addr,node->content);
    node=node->next;
  }
}
#endif
//##################################################################
