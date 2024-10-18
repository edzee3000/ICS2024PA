#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)  //这里软件(AM)实现了同步屏幕的功能, 但硬件(NEMU)尚未添加相应的支持.

void __am_gpu_init() {
  // // 这一段的测试代码只是在最开始给画面填充了一些没有意义的颜色而已，运行起来就会看到蓝绿渐变的一副画面
  // int i;
  // int w = (inl(VGACTL_ADDR)>>16);  // TODO: get the correct width
  // int h = (inl(VGACTL_ADDR)& 0xffff);  // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNC_ADDR, 1);
}
// 难道是这里有问题？？？？？？
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_wh = inl(VGACTL_ADDR);//获取VGACTL地址
  uint32_t h = screen_wh & 0xffff; //获取低4位的内容 作为高
  uint32_t w = screen_wh >> 16;  //获取高四位的内容  作为宽
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}
// void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
//   *cfg = (AM_GPU_CONFIG_T) {
//     .present = true, .has_accel = false,
//     .width = 400, .height = 300,
//     .vmemsz = 0
//   };
// }

//负责绘图的__am_gpu_fbdraw已经实现了同步屏幕的功能
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // if (ctl->sync) {
  //   outl(SYNC_ADDR, 1);
  // }
  // __am_gpu_fbdraw并没有真正实现绘图的操作，它只是把需要绘制的区域的像素点数据放到frame buffer中，
  // 也就是gpu.c当中的fb帧缓冲区，最终是在vga.c的每次设备更新中从Frame Buffer中取出数据并实现绘制

  // 在abstract-machine/am/include/amdev.h当中有个定义AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
  // 根据Flame Buffer寄存器这个定义去传入参数  用AM_##reg##_T拼接的方法定义出了寄存器的名字AM_GPU_FBDRAW_T（实际上就变成了一个struct结构体）
  // x：绘制的水平起始点
  // y：绘制的垂直起始点
  // w：绘制的矩形宽度
  // h：绘制的矩形高度
  // pixels：绘制的矩形内所有像素点的颜色，表示成二维数组就是，pixels[i][j]表示点(i+x, j+y)的颜色，这个坐标是相对整个GUI程序来说的，即GUI程序的左上角点坐标为(0, 0)
  int x=ctl->x;
  int y=ctl->y;
  int w=ctl->w;
  int h=ctl->h;
  uint32_t screen_wh = inl(VGACTL_ADDR);//获取VGACTL地址
  // uint32_t screen_h = screen_wh & 0xffff; //获取低4位的内容 作为高
  uint32_t screen_w = screen_wh >> 16;  //获取高四位的内容  作为宽
  if (!ctl->sync && (w == 0 || h == 0)) return;//若ctl->sync为false/0  并且w或h为0  则不输出直接返回
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for(int i=y;i<y+h;i++)//从竖直方向上的y轴上的y开始画
  {
    for(int j=x;j<x+w;j++)
      fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)];  //这里为什么是pixels[w*(i-y)+(j-x)]???????
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
    //abstract-machine/am/src/riscv/riscv.h当中定义的
    //static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }//写入一个字长的数据
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
