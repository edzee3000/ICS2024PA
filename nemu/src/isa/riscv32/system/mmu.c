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
#include <memory/vaddr.h>
#include <memory/paddr.h>
#include <cpu/cpu.h>



#define VA_OFFSET(addr) (addr & 0x00000FFF)   //取出低12位用于页内地址偏移offset
#define VA_VPN_0(addr)  ((addr >> 12) & 0x000003FF)  
#define VA_VPN_1(addr)  ((addr >> 22) & 0x000003FF)
#define PTE_V(item)   (item & 0x1)
#define PTE_R(item)   ((item >> 1) & 0x1)
#define PTE_W(item)   ((item >> 2) & 0x1)
#define PTE_X(item)   ((item >> 3) & 0x1)
#define PTE_U(item)   ((item >> 4) & 0x1)
#define PTE_G(item)   ((item >> 5) & 0x1)
#define PTE_A(item)   ((item >> 6) & 0x1)
#define PTE_D(item)   ((item >> 7) & 0x1)
#define PTE_PPN(item) ((item >> 12) & 0xFFFFF)

typedef uintptr_t PTE;//这里会不会有问题？？？？？？？？？？
// typedef uint32_t rtlreg_t;
////你需要理解分页地址转换的过程, 然后实现isa_mmu_check()(在nemu/src/isa/$ISA/include/isa-def.h中定义) 
// 和isa_mmu_translate()(在nemu/src/isa/$ISA/system/mmu.c中定义), 你可以查阅NEMU的ISA相关API说明文档来了解它们的行为.
//  另外由于我们不打算实现保护机制, 在isa_mmu_translate()的实现中, 你务必使用assertion检查页目录项和页表项的present/valid位, 
// 如果发现了一个无效的表项, 及时终止NEMU的运行, 否则调试将会非常困难. 这通常是由于你的实现错误引起的, 请检查实现的正确性.
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  uint32_t satp = cpu.CSRs.satp;// 从 satp 寄存器获取页表基址
  PTE page_dir_base = satp << 12;
  // 提取虚拟地址中的偏移、一级页号和二级页号
  uint32_t offset = VA_OFFSET(vaddr);//取低12位
  uint32_t vpn_1 = VA_VPN_1(vaddr);//取高10位
  uint32_t vpn_0 = VA_VPN_0(vaddr);//取中间10位
  PTE page_dir_target = page_dir_base + vpn_1 * 4;  // 计算一级页表项的地址
  word_t page_dir_target_item = paddr_read(page_dir_target, 4);  // 读取一级页表项的内容
  assert( PTE_V(page_dir_target_item) == 1 );  // 检查一级页表项是否有效，否则断言失败
  PTE page_table_base = PTE_PPN(page_dir_target_item) << 12;  // 计算二级页表的基址
  PTE page_table_target = page_table_base + vpn_0 * 4;// 计算二级页表项的地址
  word_t page_table_target_item = paddr_read(page_table_target, 4);// 读取二级页表项的内容
  assert(PTE_V(page_table_target_item) == 1);// 检查二级页表项是否有效，否则断言失败
  // 根据访存类型检查权限位
  switch (type) {
    case MEM_TYPE_IFETCH: assert(PTE_X(page_table_target_item) == 1); break;//检查可执行位（Execute）  MEM_TYPE_IFETCH在nemu/include/isa.h中enum定义
    case MEM_TYPE_READ:   assert(PTE_R(page_table_target_item) == 1); break;//检查可读位（Read）
    case MEM_TYPE_WRITE:  assert(PTE_W(page_table_target_item) == 1); break;//检查可写位（Write）
    default: assert(0); break;
  }
  // 计算物理地址  虽然PA理论上来说可能会出现34位的情况  但是右移之后取低20位
  //这个过程其实很明确，唯一让人疑惑的是PPN是一个22位的数据，最后又拼接了一个12位的地址上去，这样以来最后获得的物理地址数据岂不是34位的吗，
  // 事实上确实如此，但是二级页表里的数据其实是我们填的，所以我们只要不在二级页表中存放22位的地址就可以了。
  paddr_t ppn = PTE_PPN(page_table_target_item) << 12;
  paddr_t paddr = ppn | offset;
  // 检查计算的物理地址是否与虚拟地址相等，否则断言失败
  //由于此时Nanos-lite运行在内核的虚拟地址空间中, 而这些映射又是恒等映射, 因此NEMU的地址转换结果pa必定与va相同. 
  // 你可以把这一条件作为assertion加入到NEMU的代码中, 从而帮助你捕捉实现上的bug.
  if(paddr != vaddr)  //我靠其实这里没有问题的  不需要进行assert(paddr == vaddr);   因为不是恒等映射！！！！
  {printf("当二者不相等时type为:%d\t ppn为:%x\n",type,ppn);  printf("paddr值为: %x\t vaddr值为:%x\n",paddr,vaddr);}
  assert(vaddr >= 0x40000000 && vaddr <= 0xa1200000);
  // assert(paddr == vaddr);
  return paddr;
  // return MEM_RET_FAIL;
}


// 对于函数paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);而言：
// 对内存区间为[vaddr, vaddr + len), 类型为type的内存访问进行地址转换. 函数返回值可能为:
// pg_paddr | MEM_RET_OK: 地址转换成功, 其中pg_paddr为物理页面的地址(而不是vaddr翻译后的物理地址)
// MEM_RET_FAIL: 地址转换失败, 原因包括权限检查失败等不可恢复的原因, 一般需要抛出异常
// MEM_RET_CROSS_PAGE: 地址转换失败, 原因为访存请求跨越了页面的边界

