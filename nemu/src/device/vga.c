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

#include <common.h>
#include <device/map.h>

#define SCREEN_W (MUXDEF(CONFIG_VGA_SIZE_800x600, 800, 400))
#define SCREEN_H (MUXDEF(CONFIG_VGA_SIZE_800x600, 600, 300))
//screen_width()函数返回屏幕的宽度
static uint32_t screen_width() {
  return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).width, SCREEN_W);
}
//screen_height()函数返回屏幕的高度
static uint32_t screen_height() {
  return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).height, SCREEN_H);
}
//screen_size()函数返回屏幕的大小（高乘以宽乘以4）
static uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint32_t);
}

static void *vmem = NULL;
static uint32_t *vgactl_port_base = NULL;//  vgactl_port_base控制端口基址

#ifdef CONFIG_VGA_SHOW_SCREEN
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

//初始化屏幕函数
static void init_screen() {
  SDL_Window *window = NULL;
  char title[128];
  sprintf(title, "%s-NEMU", str(__GUEST_ISA__));
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(
      SCREEN_W * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      SCREEN_H * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      0, &window, &renderer);
  SDL_SetWindowTitle(window, title);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
  SDL_RenderPresent(renderer);
}

//更新屏幕函数
static inline void update_screen() {
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}
#else
static void init_screen() {}

static inline void update_screen() {
  io_write(AM_GPU_FBDRAW, 0, 0, vmem, screen_width(), screen_height(), true);
}
#endif
#endif



//vga更新屏幕函数
//外界调用io_write(AM_GPU_FBDRAW, ... ,true)时（最后一个参数是sync），用outb输出到抽象寄存器即可
void vga_update_screen() {
  // TODO: call `update_screen()` when the sync register is non-zero,       sync register 表示同步寄存器
  // then zero out the sync register
  // 在abstract-machine/am/src/platform/nemu/ioe/gpu.c当中有这样一个宏定义: SYNC_ADDR (VGACTL_ADDR + 4) 表示同步寄存器的地址就是VGACTL_ADDR加4  
  // 而VGACTL_ADDR的值就是vgactl_port_base（给它分配了8个字节的空间大小  前4个字节中的高2个字节存储的是宽度  低2个字节存储的是高度）
  uint32_t sync = vgactl_port_base[1];
  if (sync) {
    update_screen();
    vgactl_port_base[1] = 0;
  }
}



// VGA初始化时注册了从0xa1000000开始的一段用于映射到video memory(显存, 也叫frame buffer, 帧缓冲)的MMIO空间
// 代码只模拟了400x300x32的图形模式, 一个像素占32个bit的存储空间, R(red), G(green), B(blue), A(alpha)各占8 bit, 其中VGA不使用alpha的信息
void init_vga() {
  vgactl_port_base = (uint32_t *)new_space(8);  //VGA控制端口分配8字节的空间  这样的话可以存储2个地址了
  vgactl_port_base[0] = (screen_width() << 16) | screen_height();  //其中VGA控制端口的第一个地址4个字节存储了screen_width宽度和screen_height高度  并且高地址2个字节是宽度，低地址2个字节是高度
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("vgactl", CONFIG_VGA_CTL_PORT, vgactl_port_base, 8, NULL);
#else
  //添加VGA控制的mmio  软件AM当中gpu.c中的VGACTL_ADDR也就是vgactl的地址，在这里也就是vgactl_port_base的值  所以VGACTL_ADDR这个地址的第1个地址4个字节存储了宽高
  add_mmio_map("vgactl", CONFIG_VGA_CTL_MMIO, vgactl_port_base, 8, NULL); 
#endif

  vmem = new_space(screen_size());//创建出一块虚拟内存vmem用以存储屏幕，相当于4字节的地址*高*宽大小  
  add_mmio_map("vmem", CONFIG_FB_ADDR, vmem, screen_size(), NULL);     //添加虚拟内存的内存映射输入输出
  IFDEF(CONFIG_VGA_SHOW_SCREEN, init_screen());     //初始化屏幕
  IFDEF(CONFIG_VGA_SHOW_SCREEN, memset(vmem, 0, screen_size()));   
}
