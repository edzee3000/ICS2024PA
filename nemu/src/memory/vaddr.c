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
#include <common.h>
#include <mtrace.h>

word_t vaddr_ifetch_or_read(vaddr_t addr, int len, int type);
word_t vaddr_ifetch(vaddr_t addr, int len);
word_t vaddr_read(vaddr_t addr, int len);
void vaddr_write(vaddr_t addr, int len, word_t data);


// 在客户程序运行的过程中, 总是使用vaddr_read()和vaddr_write() (在nemu/src/memory/vaddr.c中定义)来访问模拟的内存,所以要修改 vaddr.c，type为MMU_TRANSLATE需要进行地址转换再访问

word_t vaddr_ifetch_or_read(vaddr_t addr, int len, int type) {
  int flag = isa_mmu_check(addr, len, type);
  paddr_t paddr = addr;
  switch (flag) {
    case MMU_DIRECT:
        // do nothing  什么都可以不用做直接读就行
        break;
    case MMU_TRANSLATE:
        paddr = isa_mmu_translate(addr, len, type); //isa_mmu_translate函数在nemu/src/isa/riscv32/system/mmu.c中定义的
        break;
    case MMU_FAIL:
        printf("不应该到达这里 MMU_FAIL\n");
        assert(0);
        break;
  }
  // return paddr_read(paddr, len);
  word_t content = paddr_read(paddr, len);
  ////###########在vaddr_read()中实现的mtrace转移统一到vaddr_ifetch_or_read中去实现
  IFDEF(CONFIG_MTRACE, trace_memory(addr,content));
  //#####################################
  return content;
}



word_t vaddr_ifetch(vaddr_t addr, int len) {
  //return paddr_read(addr, len);
  // return vaddr_read(addr, len);//在这里我进行了一些修改，这样的话可以将三个函数统一到vaddr_read中使用vaddr_read
  return vaddr_ifetch_or_read(addr, len, MEM_TYPE_IFETCH);
}



word_t vaddr_read(vaddr_t addr, int len) {
  // word_t content = paddr_read(addr, len);
  ////###########在vaddr_read()和vaddr_write()中进行记录实现mtrace功能即可
  // IFDEF(CONFIG_MTRACE, trace_memory(addr,content));
  //#####################################
  // return content;
  return vaddr_ifetch_or_read(addr, len, MEM_TYPE_READ);
}

 

void vaddr_write(vaddr_t addr, int len, word_t data) {
  //先在vaddr_write中写好相关内容，然后不妨写完之后再调用vaddr_read函数一次，这样就不用重复写代码了
  // paddr_write(addr, len, data);
  // IFDEF(CONFIG_MTRACE,trace_memory(addr,data));//直接调用vaddr_read函数，这样的话可以使用trace_memory
  int flag = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
  paddr_t paddr = addr;
  switch (flag) {
    case MMU_DIRECT:
        // do nothing 什么都不用做直接写就行
        break;
    case MMU_TRANSLATE:
        paddr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE); // assert in isa_mmu_translate
        break;
    case MMU_FAIL:
        printf("不应该到达这里 MMU_FAIL\n");
        assert(0);
        break;
  }
  paddr_write(paddr, len, data);
  IFDEF(CONFIG_MTRACE,trace_memory(addr,data));
}










