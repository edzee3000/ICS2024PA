#include <common.h>
#include <declaration.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};


// 首先当然是来看最简单的输出设备: 串口. 在Nanos-lite中, stdout和stderr都会输出到串口. 
// 之前你可能会通过判断fd是否为1或2, 来决定sys_write()是否写入到串口. 
// 现在有了VFS, 我们就不需要让系统调用处理函数关心这些特殊文件的情况了: 
// 我们只需要在nanos-lite/src/device.c中实现serial_write(), 然后在文件记录表中设置相应的写函数, 就可以实现上述功能了.
// 由于串口是一个字符设备, 对应的字节序列没有"位置"的概念, 因此serial_write()中的offset参数可以忽略.
// 另外Nanos-lite也不打算支持stdin的读入, 因此在文件记录表中设置相应的报错函数即可.
size_t serial_write(const void *buf, size_t offset, size_t len) {
  //这里我们可以默认串口写函数得到offset为0
  for(size_t i=0;i<len;i++){putch(*( (char*)buf+i) );}
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  // 实现events_read()(在nanos-lite/src/device.c中定义), 把事件写入到buf中, 最长写入len字节, 然后返回写入的实际长度. 
  // 其中按键名已经在字符串数组names中定义好了, 你需要借助IOE的API来获得设备的输入. 另外, 若当前没有有效按键, 则返回0即可.
  AM_INPUT_KEYBRD_T key = io_read(AM_INPUT_KEYBRD);//按键信息对系统来说本质上就是到来了一个事件
  if (key.keycode == AM_KEY_NONE) {*(char*)buf = '\0';return 0;}
  int buflen=snprintf(buf, len, "%s %s\n", key.keydown ? "kd":"ku", keyname[key.keycode]);//按键名称与AM中的定义的按键名相同, 均为大写. 此外, 一个事件以换行符\n结束.
  printf("buf内容为:%s",buf);//这里就不用多加一个\n了因为上一条代码已经用\n分隔开来了
  return buflen;
}

//显存屏幕大小信息读取
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  //实现dispinfo_read()(在nanos-lite/src/device.c中定义), 按照约定将文件的len字节写到buf中(我们认为这个文件不支持lseek, 可忽略offset).
  AM_GPU_CONFIG_T gpu=io_read(AM_GPU_CONFIG);
  return snprintf((char*)buf,len,"屏幕宽度:%d 屏幕高度:%d\n",gpu.width, gpu.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  //fb_write()(在nanos-lite/src/device.c中定义), 用于把buf中的len字节写到屏幕上offset处. 你需要先从offset计算出屏幕上的坐标, 然后调用IOE来进行绘图. 另外我们约定每次绘图后总是马上将frame buffer中的内容同步到屏幕上.
  AM_GPU_CONFIG_T gpu = io_read(AM_GPU_CONFIG);
  int screen_width=gpu.width;
  offset/=sizeof(uint32_t);len/=sizeof(uint32_t);//因为按照一个像素就是4个字节的存储
  int y = offset / screen_width;  //确认行数
  int x = offset % screen_width;//确认列数
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len, 1, true);//这里1表示高为1，也就是说只输出一行
  // draw_rect((uint32_t*)buf, x,y, len/4, 1);
  printf("screen_width:%u\toffset:%u\tlen:%u\tx:%u\ty:%u\n",screen_width,offset,len,x,y);
  //每次只能写一行的原因是文件系统的接口限制了我们无法将画布宽高也同时传进write中，fb的写操作只会根据屏幕大小，行优先地写入帧缓冲中
  return len*sizeof(uint32_t);
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
