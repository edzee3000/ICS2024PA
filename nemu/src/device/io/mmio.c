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
// 端口映射I/O把端口号作为I/O指令的一部分, 这种方法很简单, 但同时也是它最大的缺点. 指令集为了兼容已经开发的程序, 是只能添加但不能修改的. 
// 这意味着, 端口映射I/O所能访问的I/O地址空间的大小, 在设计I/O指令的那一刻就已经决定下来了. 所谓I/O地址空间, 
// 其实就是所有能访问的设备的地址的集合. 随着设备越来越多, 功能也越来越复杂, I/O地址空间有限的端口映射I/O已经逐渐不能满足需求了. 
// 有的设备需要让CPU访问一段较大的连续存储空间, 如VGA的显存, 24色加上Alpha通道的1024x768分辨率的显存就需要3MB的编址范围. 
// 于是内存映射I/O(memory-mapped I/O, MMIO)应运而生.

// 内存映射I/O这种编址方式非常巧妙, 它是通过不同的物理内存地址给设备编址的. 这种编址方式将一部分物理内存的访问"重定向"到I/O地址空间中, 
// CPU尝试访问这部分物理内存的时候, 实际上最终是访问了相应的I/O设备, CPU却浑然不知. 这样以后, CPU就可以通过普通的访存指令来访问设备. 
// 这也是内存映射I/O得天独厚的好处: 物理内存的地址空间和CPU的位宽都会不断增长, 内存映射I/O从来不需要担心I/O地址空间耗尽的问题. 
// 从原理上来说, 内存映射I/O唯一的缺点就是, CPU无法通过正常渠道直接访问那些被映射到I/O地址空间的物理内存了. 
// 但随着计算机的发展, 内存映射I/O的唯一缺点已经越来越不明显了: 现代计算机都已经是64位计算机, 物理地址线都有48根, 
// 这意味着物理地址空间有256TB这么大, 从里面划出3MB的地址空间给显存, 根本就是不痛不痒. 正因为如此, 
// 内存映射I/O成为了现代计算机主流的I/O编址方式: RISC架构只提供内存映射I/O的编址方式, 而PCI-e, 网卡, x86的APIC等主流设备, 
// 都支持通过内存映射I/O来访问.

#include <device/map.h>
#include <memory/paddr.h>
#include <dtrace.h>

#define NR_MAP 16

static IOMap maps[NR_MAP] = {};
static int nr_map = 0;



//获取内存映射输入输出map函数
static IOMap* fetch_mmio_map(paddr_t addr) {
  int mapid = find_mapid_by_addr(maps, nr_map, addr);//通过地址去找到map映射的id
  return (mapid == -1 ? NULL : &maps[mapid]);
}





//报告内存映射输入输出重叠了
static void report_mmio_overlap(const char *name1, paddr_t l1, paddr_t r1,
    const char *name2, paddr_t l2, paddr_t r2) {
  panic("MMIO region %s@[" FMT_PADDR ", " FMT_PADDR "] is overlapped "
               "with %s@[" FMT_PADDR ", " FMT_PADDR "]", name1, l1, r1, name2, l2, r2);
}





/* device interface 初始化添加设备接口  添加内存映射输入输出的接口  serial串口、timer时钟等等都要用它*/
void add_mmio_map(const char *name, paddr_t addr, void *space, uint32_t len, io_callback_t callback) {
  //限制最多只能有NR_MAP个设备接口（map方式）
  assert(nr_map < NR_MAP);//如果自己添加的设备接口数目超过NR_MAP的话会assert报错
  paddr_t left = addr, right = addr + len - 1;//设备接口需要的长度为len，比如serial串口设备就需要8字节的内存大小
  if (in_pmem(left) || in_pmem(right)) {  //我们需要保证设备接口不在我们真实的物理地址里面，因为可能会误访问  因此需要判断  如果在里面的话就panic报错
    report_mmio_overlap(name, left, right, "pmem", PMEM_LEFT, PMEM_RIGHT);
  }
  for (int i = 0; i < nr_map; i++) {//重新检查所有的所有的设备映射map之间是否存在内存重叠的问题
    if (left <= maps[i].high && right >= maps[i].low) {
      report_mmio_overlap(name, left, right, maps[i].name, maps[i].low, maps[i].high);
    }
  }
  //如果一切检查都没有问题的话就创建新的map并且赋值给新id的map
  maps[nr_map] = (IOMap){ .name = name, .low = addr, .high = addr + len - 1,
    .space = space, .callback = callback };//新的map占用内存范围为 [low,high]  之间 
  Log("Add mmio map '%s' at [" FMT_PADDR ", " FMT_PADDR "]",
      maps[nr_map].name, maps[nr_map].low, maps[nr_map].high);//这里有一个Log语句表示添加内存映射输入输出map映射
  //更新设备接口的个数
  nr_map ++;
}





/* bus interface 总线接口*/
word_t mmio_read(paddr_t addr, int len) {
  //########在这里添加dtrace语句####################################################################################
  IOMap* map=fetch_mmio_map(addr); //获得addr对应的map映射，  通过索引id去寻找，  比如如果addr在serial的索引范围之内就返回serial串口对应的map
  IFDEF(CONFIG_DTRACE,trace_dread(addr,len,map));
  return map_read(addr, len, map);
}

void mmio_write(paddr_t addr, int len, word_t data) {
  //########在这里添加dtrace语句####################################################################################
  IOMap* map=fetch_mmio_map(addr);
  IFDEF(CONFIG_DTRACE,trace_dwrite(addr,len,data,map));
  map_write(addr, len, data, map);
}
