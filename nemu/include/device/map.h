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

#ifndef __DEVICE_MAP_H__
#define __DEVICE_MAP_H__

#include <cpu/difftest.h>

typedef void(*io_callback_t)(uint32_t, int, bool);
uint8_t* new_space(int size);

typedef struct {
  const char *name;//名字
  // we treat ioaddr_t as paddr_t here  我们将输入输出地址在这里视为物理地址
  paddr_t low;//映射的起始地址
  paddr_t high;//映射的结束地址
  void *space;//映射的目标空间
  io_callback_t callback;//回调函数
} IOMap;
//框架代码为映射定义了一个结构体类型IOMap(在nemu/include/device/map.h中定义), 包括名字, 映射的起始地址和结束地址, 映射的目标空间, 
// 以及一个回调函数. 然后在nemu/src/device/io/map.c实现了映射的管理, 包括I/O空间的分配及其映射, 还有映射的访问接口.

static inline bool map_inside(IOMap *map, paddr_t addr) {
  return (addr >= map->low && addr <= map->high);
}

static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
  int i;
  for (i = 0; i < size; i ++) {
    if (map_inside(maps + i, addr)){ //如果addr是在map映射的范围里面的话返回对应的map的id
      difftest_skip_ref();
      return i;
    }
  }
  return -1;
}

void add_pio_map(const char *name, ioaddr_t addr,
        void *space, uint32_t len, io_callback_t callback);
void add_mmio_map(const char *name, paddr_t addr,
        void *space, uint32_t len, io_callback_t callback);

word_t map_read(paddr_t addr, int len, IOMap *map);
void map_write(paddr_t addr, int len, word_t data, IOMap *map);

#endif
