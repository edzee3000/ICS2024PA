#include <am.h>
#include <klib-macros.h>

#include <amdev.h>
#include <am-origin.h>
// void __am_timer_init();
// void __am_gpu_init();
// void __am_audio_init();
// void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
// void __am_timer_rtc(AM_TIMER_RTC_T *);
// void __am_timer_uptime(AM_TIMER_UPTIME_T *);
// void __am_gpu_config(AM_GPU_CONFIG_T *);
// void __am_gpu_status(AM_GPU_STATUS_T *);
// void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
// void __am_audio_config(AM_AUDIO_CONFIG_T *);
// void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
// void __am_audio_status(AM_AUDIO_STATUS_T *);
// void __am_audio_play(AM_AUDIO_PLAY_T *);
// void __am_disk_config(AM_DISK_CONFIG_T *cfg);
// void __am_disk_status(AM_DISK_STATUS_T *stat);
// void __am_disk_blkio(AM_DISK_BLKIO_T *io);

// static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
// static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
// static void __am_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }
// static void __am_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }


typedef void (*handler_t)(void *buf);
//下面lut里面的不是设备寄存器而是抽象寄存器
static void *lut[128] = { //lut的意思是 "lookup table"（查找表)
  // [AM_TIMER_CONFIG] = __am_timer_config,
  // [AM_TIMER_RTC   ] = __am_timer_rtc,
  // [AM_TIMER_UPTIME] = __am_timer_uptime,
  // [AM_INPUT_CONFIG] = __am_input_config,
  // [AM_INPUT_KEYBRD] = __am_input_keybrd,  //定义了一个AM_INPUT_KEYBRD抽象寄存器
  // [AM_GPU_CONFIG  ] = __am_gpu_config,
  // [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,  //__am_gpu_fbdraw这个函数在nemu里面就是用来实现同步屏幕的功能
  // [AM_GPU_STATUS  ] = __am_gpu_status,
  // [AM_UART_CONFIG ] = __am_uart_config,
  // [AM_AUDIO_CONFIG] = __am_audio_config,
  // [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
  // [AM_AUDIO_STATUS] = __am_audio_status,
  // [AM_AUDIO_PLAY  ] = __am_audio_play,
  // [AM_DISK_CONFIG ] = __am_disk_config,
  // [AM_DISK_STATUS ] = __am_disk_status,
  // [AM_DISK_BLKIO  ] = __am_disk_blkio,
  // [AM_NET_CONFIG  ] = __am_net_config,
};

static void fail(void *buf) { panic("access nonexist register"); }

bool ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  //这里ioe_init的实现直接照搬abstract-machine/am/src/platform/nemu/ioe/ioe.c当中的实现会不会有问题？？？
  // __am_gpu_init();  
  // __am_timer_init();
  // __am_audio_init();
  return true;
}

// void ioe_read (int reg, void *buf) { int fd = open("/dev/am_ioe", 0, 0); lseek(fd, reg, SEEK_SET);  read(fd, buf, 0);close(fd); }
// void ioe_write(int reg, void *buf) {  int fd = open("/dev/am_ioe", 0, 0);lseek(fd, reg, SEEK_SET);write(fd, buf, 0);close(fd);}
 
void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
// void ioe_read (int reg, void *buf) {}
// void ioe_write(int reg, void *buf) {}


