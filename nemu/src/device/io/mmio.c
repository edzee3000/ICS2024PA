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
