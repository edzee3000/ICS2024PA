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
#include <memory/host.h>
#include <memory/vaddr.h>
#include <device/map.h>

#define IO_SPACE_MAX (2 * 1024 * 1024)

static uint8_t *io_space = NULL;
static uint8_t *p_space = NULL;

//开辟新空间函数
uint8_t* new_space(int size) {
  uint8_t *p = p_space;
  // page aligned;  页面对齐
  size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
  p_space += size;
  assert(p_space - io_space < IO_SPACE_MAX);
  return p;
}

// 用于检查给定的地址 addr 是否在指定的内存映射 map 的有效范围内。
// 这个函数通常用于模拟器、虚拟机或者操作系统的内存管理单元（MMU）中，以确保对内存的访问是合法的，没有越界。
static void check_bound(IOMap *map, paddr_t addr) {
  //假设map为NULL或者addr不在[map->low,map->high]的范围里面的话那就报错
  if (map == NULL) {
    Assert(map != NULL, "address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
  } else {
    Assert(addr <= map->high && addr >= map->low,
        "address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, map->name, map->low, map->high, cpu.pc);
  }
}
//唤醒callback回调函数，在serial.c串口文件中有使用过用以判断是否是写而非读
static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
  if (c != NULL) { c(offset, len, is_write); }  //执行callback回调函数
}

void init_map() {
  io_space = malloc(IO_SPACE_MAX);
  assert(io_space);
  p_space = io_space;
}



//其中map_read()和map_write()用于将地址addr映射到map所指示的目标空间, 并进行访问. 访问时, 可能会触发相应的回调函数, 
// 对设备和目标空间的状态进行更新. 由于NEMU是单线程程序, 因此只能串行模拟整个计算机系统的工作, 每次进行I/O读写的时候, 
// 才会调用设备提供的回调函数(callback). 基于这两个API, 我们就可以很容易实现端口映射I/O和内存映射I/O的模拟了.
//map_read 函数用于从指定的内存映射区域读取数据。
word_t map_read(paddr_t addr, int len, IOMap *map) {
  assert(len >= 1 && len <= 8);//检查len的长度要在[1,8]之内
  check_bound(map, addr);//检查addr是否越界（不在device设备的内存映射范围里面）
  paddr_t offset = addr - map->low; // addr相对于device的映射偏移量
  invoke_callback(map->callback, offset, len, false); // 准备要读取的数据  根据本次映射的回调函数  根据不同的设备准备了不同的回调函数
  word_t ret = host_read(map->space + offset, len);  //  真正从 map->space + offset 这个地址处读取长度为len的数据
  //事实上真正的物理地址不是addr，addr只是一个虚拟地址，真正的物理地址应该是 addr - (map->low) + (map->space)  
  return ret;
}
//内存映射I/O的模拟是类似的, paddr_read()和paddr_write()会判断地址addr落在物理内存空间还是设备空间, 若落在物理内存空间, 
// 就会通过pmem_read()和pmem_write()来访问真正的物理内存; 否则就通过map_read()和map_write()来访问相应的设备. 
// 从这个角度来看, 内存和外设在CPU来看并没有什么不同, 只不过都是一个字节编址的对象而已.
//map_write 函数用于向指定的内存映射区域写入数据
void map_write(paddr_t addr, int len, word_t data, IOMap *map) {
  assert(len >= 1 && len <= 8);
  check_bound(map, addr);
  paddr_t offset = addr - map->low;
  host_write(map->space + offset, len, data);
  invoke_callback(map->callback, offset, len, true);
}
