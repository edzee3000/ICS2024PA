#include <am.h>
#include <klib-macros.h>

void __am_timer_init();
void __am_gpu_init();
void __am_audio_init();
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);
void __am_disk_config(AM_DISK_CONFIG_T *cfg);
void __am_disk_status(AM_DISK_STATUS_T *stat);
void __am_disk_blkio(AM_DISK_BLKIO_T *io);

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __am_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }
static void __am_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }

typedef void (*handler_t)(void *buf);
//下面lut里面的不是设备寄存器而是抽象寄存器
static void *lut[128] = { //lut的意思是 "lookup table"（查找表)
  [AM_TIMER_CONFIG] = __am_timer_config,
  [AM_TIMER_RTC   ] = __am_timer_rtc,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,  //定义了一个AM_INPUT_KEYBRD抽象寄存器
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,  //__am_gpu_fbdraw这个函数在nemu里面就是用来实现同步屏幕的功能
  [AM_GPU_STATUS  ] = __am_gpu_status,
  [AM_UART_CONFIG ] = __am_uart_config,
  [AM_AUDIO_CONFIG] = __am_audio_config,
  [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
  [AM_AUDIO_STATUS] = __am_audio_status,
  [AM_AUDIO_PLAY  ] = __am_audio_play,
  [AM_DISK_CONFIG ] = __am_disk_config,
  [AM_DISK_STATUS ] = __am_disk_status,
  [AM_DISK_BLKIO  ] = __am_disk_blkio,
  [AM_NET_CONFIG  ] = __am_net_config,
};

static void fail(void *buf) { panic("access nonexist register"); }

//用于进行IOE相关的初始化操作
bool ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  __am_gpu_init();  
  __am_timer_init();
  __am_audio_init();
  return true;
  //初始化gpu、timer、audio等
}

// ioe_read()和ioe_write()都是通过抽象寄存器的编号索引到一个处理函数, 然后调用它. 处理函数的具体功能和寄存器编号相关
void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); } //从编号为reg的寄存器中读出内容到缓冲区buf中
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); } //往编号为reg寄存器中写入缓冲区buf中的内容
// 通过ioe_write调用对应的lut "lookup table"（查找表)  然后执行am中对应的函数

// 需要注意的是, 这里的reg寄存器并不是上文讨论的设备寄存器, 因为设备寄存器的编号是架构相关的. 
// 在IOE中, 我们希望采用一种架构无关的"抽象寄存器", 这个reg其实是一个功能编号,
// 我们约定在不同的架构中, 同一个功能编号的含义也是相同的, 这样就实现了设备寄存器的抽象.


// 设备访问的具体实现是架构相关的, 比如NEMU的VGA显存位于物理地址区间[0xa1000000, 0xa1080000), 
// 但对native的程序来说, 这是一个不可访问的非法区间, 因此native程序需要通过别的方式来实现类似的功能. 
// 自然地, 设备访问这一架构相关的功能, 应该归入AM中


/*
举个例子  在am-kernels/kernels/typing-game/game.c中的io_write函数
io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);  <- 
<- io_write(reg, ...) \
     ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ }; \
        ioe_write(reg, &__io_param); })
<- void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); } 
*/











