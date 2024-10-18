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
#include <device/alarm.h>
#include <utils.h>

//rtc 是 "Real-Time Clock" 的缩写，意为实时时钟
//实时时钟是一种硬件设备，用于维护系统的当前日期和时间，即使在计算机关机时也能通过电池供电保持时间的准确性。
//rtc_port_base 定义了实时时钟设备寄存器映射到系统内存中的起始地址。
//这个基地址用于访问实时时钟的控制寄存器和数据寄存器，以便程序可以读取当前时间、设置时间或执行其他与时间相关的操作。
static uint32_t *rtc_port_base = NULL;
//rtc_io_handler是一个回调函数
static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
  assert(offset == 0 || offset == 4);
  if (!is_write && offset == 4) {//如果是read并且偏移量为4的话
    // 注意这里有个trick有一个判断offset==4才会执行下面的语句
    // 意思是获取一次系统时间会触发两次这个回调函数，加一个判断避免重复get_time
    // 在上层am当中的__am_timer_uptime中inl的顺序应该是先读高32位，获取到当前系统时间，然后再读低32位
    uint64_t us = get_time();//获得时间  而且这个时间是相对于boot_time而言的时间
    rtc_port_base[0] = (uint32_t)us; //存储低32位
    rtc_port_base[1] = us >> 32;  //存储高32位
  }
}
// nemu/src/device/io/map.c中的  map_read()函数   当offset为4的时候，表示此时 


#ifndef CONFIG_TARGET_AM
static void timer_intr() {//虚拟环境中唤醒时间指令    处理计时器中断的回调函数
  if (nemu_state.state == NEMU_RUNNING) {//当模拟器处于运行状态时，它会触发一个设备中断
    extern void dev_raise_intr();
    dev_raise_intr();
  }
}
#endif

void init_timer() {
  rtc_port_base = (uint32_t *)new_space(8);
  // 给rtc开辟8字节的空间用于存储rtc端口的内容  
  // 在这个情境下面表示的是 rtc_port_base[0]存储64位时间的低32位  rtc_port_base[1]存储64位时间的高32位
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("rtc", CONFIG_RTC_PORT, rtc_port_base, 8, rtc_io_handler);
#else
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
#endif
  IFNDEF(CONFIG_TARGET_AM, add_alarm_handle(timer_intr));
}
 