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
// 设备的工作原理其实没什么神秘的. 你会在不久的将来在数字电路实验中看到键盘控制器模块和VGA控制器模块相关的verilog代码. 
// 噢, 原来这些设备也一样是个数字电路! 事实上, 只要向设备发送一些有意义的数字信号, 设备就会按照这些信号的含义来工作. 
// 让一些信号来指导设备如何工作, 这不就像"程序的指令指导CPU如何工作"一样吗? 恰恰就是这样! 设备也有自己的状态寄存器(相当于CPU的寄存器), 
// 也有自己的功能部件(相当于CPU的运算器). 当然不同的设备有不同的功能部件, 例如键盘有一个把按键的模拟信号转换成扫描码的部件, 
// 而VGA则有一个把像素颜色信息转换成显示器模拟信号的部件. 控制设备工作的信号称为"命令字", 可以理解成"设备的指令", 
// 设备的工作就是负责接收命令字, 并进行译码和执行... 你已经知道CPU的工作方式, 这一切对你来说都太熟悉了.

// 在程序看来, 访问设备 = 读出数据 + 写入数据 + 控制状态.


#include <common.h>
#include <utils.h>
#include <device/alarm.h>
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>
#endif

void init_map();
void init_serial();
void init_timer();
void init_vga();
void init_i8042();
void init_audio();
void init_disk();
void init_sdcard();
void init_alarm();

void send_key(uint8_t, bool);
void vga_update_screen();

//另一方面, cpu_exec()在执行每条指令之后就会调用device_update()函数, 这个函数首先会检查距离上次设备更新是否已经超过一定时间, 
// 若是, 则会尝试刷新屏幕, 并进一步检查是否有按键按下/释放, 以及是否点击了窗口的X按钮; 否则则直接返回, 
// 避免检查过于频繁, 因为上述事件发生的频率是很低的.
void device_update() {
  static uint64_t last = 0;
  uint64_t now = get_time();
  if (now - last < 1000000 / TIMER_HZ) {
    return;
  }
  last = now;

  IFDEF(CONFIG_HAS_VGA, vga_update_screen());//如果有VGA的话就使用 vga更新屏幕函数

#ifndef CONFIG_TARGET_AM
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        nemu_state.state = NEMU_QUIT;
        break;
#ifdef CONFIG_HAS_KEYBOARD
      // If a key was pressed
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        uint8_t k = event.key.keysym.scancode;
        bool is_keydown = (event.key.type == SDL_KEYDOWN);
        send_key(k, is_keydown);
        break;
      }
#endif
      default: break;
    }
  }
#endif
}

//NEMU使用SDL库来实现设备的模拟, nemu/src/device/device.c含有和SDL库相关的代码.

void sdl_clear_event_queue() {
#ifndef CONFIG_TARGET_AM
  SDL_Event event;
  while (SDL_PollEvent(&event));
#endif
}


// init_device()函数主要进行以下工作:
// 调用init_map()进行初始化.
// 对上述设备进行初始化, 其中在初始化VGA时还会进行一些和SDL相关的初始化工作, 包括创建窗口, 设置显示模式等;
// 然后会进行定时器(alarm)相关的初始化工作. 定时器的功能在PA4最后才会用到, 目前可以忽略它.

void init_device() {
  //如果nemu的上层目标是am的话就调用ioe_init函数
  IFDEF(CONFIG_TARGET_AM, ioe_init());
  init_map();//调用init_map()进行初始化映射的操作  在nemu/src/device/io/map.c文件里面
  //如果有对应的设备定义的话就调用对应设备的初始化函数（因为声卡是选做内容因此暂时还没有config定义）
  IFDEF(CONFIG_HAS_SERIAL, init_serial());//
  IFDEF(CONFIG_HAS_TIMER, init_timer());
  IFDEF(CONFIG_HAS_VGA, init_vga());
  IFDEF(CONFIG_HAS_KEYBOARD, init_i8042());
  IFDEF(CONFIG_HAS_AUDIO, init_audio());
  IFDEF(CONFIG_HAS_DISK, init_disk());
  IFDEF(CONFIG_HAS_SDCARD, init_sdcard());

  IFNDEF(CONFIG_TARGET_AM, init_alarm());//定时器的功能在PA4最后才会用到, 目前可以忽略它.
}
