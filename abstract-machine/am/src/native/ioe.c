#include <am.h>
#include <klib-macros.h>

bool __am_has_ioe = false;
static bool ioe_init_done = false;

void __am_timer_init();
void __am_gpu_init();
void __am_input_init();
void __am_uart_init();
void __am_audio_init();
void __am_disk_init();
void __am_input_config(AM_INPUT_CONFIG_T *);
void __am_timer_config(AM_TIMER_CONFIG_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_uart_config(AM_UART_CONFIG_T *);
void __am_uart_tx(AM_UART_TX_T *);
void __am_uart_rx(AM_UART_RX_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);
void __am_disk_config(AM_DISK_CONFIG_T *cfg);
void __am_disk_status(AM_DISK_STATUS_T *stat);
void __am_disk_blkio(AM_DISK_BLKIO_T *io);
static void __am_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __am_timer_config,
  [AM_TIMER_RTC   ] = __am_timer_rtc,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,
  [AM_GPU_STATUS  ] = __am_gpu_status,
  [AM_UART_CONFIG ] = __am_uart_config,
  [AM_UART_TX     ] = __am_uart_tx,
  [AM_UART_RX     ] = __am_uart_rx,
  [AM_AUDIO_CONFIG] = __am_audio_config,
  [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
  [AM_AUDIO_STATUS] = __am_audio_status,
  [AM_AUDIO_PLAY  ] = __am_audio_play,
  [AM_DISK_CONFIG ] = __am_disk_config,
  [AM_DISK_STATUS ] = __am_disk_status,
  [AM_DISK_BLKIO  ] = __am_disk_blkio,
  [AM_NET_CONFIG  ] = __am_net_config,
};

//用于进行IOE相关的初始化操作.
bool ioe_init() {
  panic_on(cpu_current() != 0, "call ioe_init() in other CPUs");
  panic_on(ioe_init_done, "double-initialization");
  __am_has_ioe = true;
  return true;
}

static void fail(void *buf) { panic("access nonexist register"); }

void __am_ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  __am_timer_init();
  __am_gpu_init();
  __am_input_init();
  __am_uart_init();
  __am_audio_init();
  __am_disk_init();
  ioe_init_done = true;
}

static void do_io(int reg, void *buf) {
  if (!ioe_init_done) {
    __am_ioe_init();//如果没有对ioe初始化的话那就在这里进行初始化
  }
  ((handler_t)lut[reg])(buf);
}

void ioe_read (int reg, void *buf) { do_io(reg, buf); }//从编号为reg的寄存器中读出内容到缓冲区buf中
void ioe_write(int reg, void *buf) { do_io(reg, buf); }//往编号为reg寄存器中写入缓冲区buf中的内容


// 设备访问的具体实现是架构相关的, 比如NEMU的VGA显存位于物理地址区间[0xa1000000, 0xa1080000), 
// 但对native的程序来说, 这是一个不可访问的非法区间, 因此native程序需要通过别的方式来实现类似的功能. 
// 自然地, 设备访问这一架构相关的功能, 应该归入AM中