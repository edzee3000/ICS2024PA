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
    // 假设根据am中的timer.c先  uint32_t low = inl(RTC_ADDR); 再 uint32_t high = inl(RTC_ADDR+4);  也就是先获取低字节再获取高字节时间
    // 那么硬件nemu根据am中的命令，所获得的指令应该是先offset==0再offset==4，  
    // 那么此时根据offset=0不获得当前的时间，即不更新rtc_port_base[0]低32位，获取到的是上一次__am_timer_uptime得到的系统时间的低32位
    // 等到要获取高字节时间的时候，此时的offset==4，开始获得当前时间  并且将获得的时间更新到rtc_port_base里面去
    // 在native里面到时没啥问题，但是在nemu里面就要命了，因为效率不是很高，其实是差了一拍的……
    uint64_t us = get_time();//获得当前时间  而且这个时间是相对于boot_time而言的时间
    rtc_port_base[0] = (uint32_t)us; //存储低32位
    rtc_port_base[1] = us >> 32;  //存储高32位
  }
}
// nemu/src/device/io/map.c中的  map_read()函数   当offset为4的时候，表示此时 


#ifndef CONFIG_TARGET_AM
static void timer_intr() {//虚拟环境中唤醒时间指令    处理计时器中断的回调函数
  //在NEMU中, 我们只需要添加时钟中断这一种中断就可以了. 由于只有一种中断, 我们也不需要通过中断控制器进行中断的管理, 
  // 直接让时钟中断连接到CPU的INTR引脚即可. 而对于时钟中断的中断号, 不同的ISA有不同的约定. 
  // 时钟中断通过nemu/src/device/timer.c中的timer_intr()触发, 每10ms触发一次. 触发后, 
  // 会调用dev_raise_intr()函数(在nemu/src/device/intr.c中定义)
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
 