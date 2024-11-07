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
  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
