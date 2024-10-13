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

#include <mtrace.h>

word_t vaddr_ifetch(vaddr_t addr, int len);
word_t vaddr_read(vaddr_t addr, int len);
void vaddr_write(vaddr_t addr, int len, word_t data);


word_t vaddr_ifetch(vaddr_t addr, int len) {
  //return paddr_read(addr, len);
  return vaddr_read(addr, len);//在这里我进行了一些修改，这样的话可以将三个函数统一到vaddr_read中使用vaddr_read
}



word_t vaddr_read(vaddr_t addr, int len) {
  word_t content = paddr_read(addr, len);
  ////###########在vaddr_read()和vaddr_write()中进行记录实现mtrace功能即可
  IFDEF(CONFIG_MTRACE, trace_memory(addr,content));
  //#####################################
  return content;
}

 

void vaddr_write(vaddr_t addr, int len, word_t data) {
  //先在vaddr_write中写好相关内容，然后不妨写完之后再调用vaddr_read函数一次，这样就不用重复写代码了
  paddr_write(addr, len, data);
  IFDEF(CONFIG_MTRACE,trace_memory(addr,data));//直接调用vaddr_read函数，这样的话可以使用trace_memory
}










