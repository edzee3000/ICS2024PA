/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>


//#################定义一个MTRACE_NODE的作为记录MTRACE节点##############
#ifdef CONFIG_MTRACE
typedef struct MTRACE_NODE{
  uint32_t addr;
  uint32_t content;
  struct MTRACE_NODE *next;
}MTRACE_NODE;
MTRACE_NODE *head_mtrace=NULL;
MTRACE_NODE *tail_mtrace;
#endif
//##################################################################




word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}



word_t vaddr_read(vaddr_t addr, int len) {
  word_t content = paddr_read(addr, len);
  ////###########在vaddr_read()和vaddr_write()中进行记录实现mtrace功能即可
  //IFDEF(CONFIG_MTRACE, )
  //#####################################
  return content;
}



void vaddr_write(vaddr_t addr, int len, word_t data) {
  //先在vaddr_write中写好相关内容，然后不妨写完之后再调用vaddr_read函数一次，这样就不用重复写代码了
  paddr_write(addr, len, data);
}









//下面都是MTRACE相关函数实现
void trace_memory()
{

}


