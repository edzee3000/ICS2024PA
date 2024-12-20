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



void dev_raise_intr() {
  //在NEMU中, 我们只需要添加时钟中断这一种中断就可以了. 由于只有一种中断, 我们也不需要通过中断控制器进行中断的管理, 
  // 直接让时钟中断连接到CPU的INTR引脚即可. 而对于时钟中断的中断号, 不同的ISA有不同的约定. 
  // 时钟中断通过nemu/src/device/timer.c中的timer_intr()触发, 每10ms触发一次. 触发后, 
  // 会调用dev_raise_intr()函数(在nemu/src/device/intr.c中定义)
  //   在dev_raise_intr()中将INTR引脚设置为高电平.
    cpu.INTR=true;
}
