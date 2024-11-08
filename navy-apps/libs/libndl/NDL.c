#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sys/time.h"
#include <time.h>
#include "assert.h"

static int evtdev = -1;
static int fbdev = -1;
//屏幕大小
static int screen_w = 0, screen_h = 0;
//画布大小
static int canvas_w=0,canvas_h=0;
//相对于屏幕左上角的画布位置坐标
static int canvas_x=0,canvas_y=0;

enum{SDL_INIT_TIMER,SDL_INIT_AUDIO,SDL_INIT_VIDEO,
SDL_INIT_JOYSTICK,SDL_INIT_HAPTIC,SDL_INIT_GAMECONTROLLER,
SDL_INIT_EVENTS,SDL_INIT_NOPARACHUTE,SDL_INIT_EVERYTHING};

uint32_t NDL_GetTicks() {
  //SDL_GetTicks这个函数功能返回自从SDL库初始化到现在经过的毫秒值
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


int open (const char *file, int flags, ...);

// 读出一条事件信息, 将其写入`buf`中, 最长写入`len`字节
// 若读出了有效的事件, 函数返回1, 否则返回0
int NDL_PollEvent(char *buf, int len) {
  // 另一个输入设备是键盘, 按键信息对系统来说本质上就是到来了一个事件. 一种简单的方式是把事件以文本的形式表现出来, 我们定义以下两种事件,
  // 按下按键事件, 如kd RETURN表示按下回车键
  // 松开按键事件, 如ku A表示松开A键
  // 按键名称与AM中的定义的按键名相同, 均为大写. 此外, 一个事件以换行符\n结束.
  // 我们采用文本形式来描述事件有两个好处, 首先文本显然是一种字节序列, 这使得事件很容易抽象成文件; 
  // 此外文本方式使得用户程序可以容易可读地解析事件的内容. Nanos-lite和Navy约定, 
  // 上述事件抽象成一个特殊文件/dev/events, 它需要支持读操作, 用户程序可以从中读出按键事件, 
  // 但它不必支持lseek, 因为它是一个字符设备.
  int fd = open("/dev/events", 0 , 0);//第一个区别在于fopen属于缓冲文件系统，fopen, fclose, fread, fwrite, fseek等都有缓冲区，而open属于非缓冲文件系统，相关文件操作的函数有open, close, read, write, lseek，由于键盘是字符设备，而且写入速度（用户键盘输入）十分慢，不需要缓冲区，因此选择后者
  int readlen=read(fd,buf,len);
  assert(close(fd)==0);
  return readlen ==0 ? 0:1 ;
}

// 打开一张(*w) X (*h)的画布
// 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小
void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  //这里从/proc/dispinfo 中解析出屏幕的宽和高，然后赋值， 根据屏幕的值和画布的宽高算出画布的原点(相对于屏幕左上角的位置坐标)
  int buf_size = 128;
  char * buf = (char *)malloc(buf_size * sizeof(char));
  int fd = open("/proc/dispinfo", 0, 0);
  int readlen=read(fd,buf,buf_size);
  assert(readlen < buf_size);//为了要保证读取的长度要小于buf缓冲区的大小
  assert(close(fd)==0); 
  // sscanf(buf, "屏幕宽度:%d 屏幕高度:%d\n", &screen_w, &screen_h);

  int i=0,width=0,height=0;
  for(int j=0;j<2;j++){//循环2次分别读取buf中的宽度与高度
    for (; i < buf_size; i++) {if (buf[i] == ':') { i++; break;}}
    for (; i < buf_size; i++) {if (buf[i] >= '0' && buf[i] <= '9'){break;} printf("冒号后面不是数字\n");assert(0);} //检查当前字符是否是数字字符。如果是，它跳出循环以开始解析宽度值。
    for (; i < buf_size; i++) 
    {if (buf[i] >= '0' && buf[i] <= '9') {switch (j)
    {case 0:  width = width * 10 + buf[i] - '0'; break;
     case 1:  height = height * 10 + buf[i] - '0';break;
     default:break;}} //检查当前字符是否是数字字符。如果是，它将当前字符的数字值添加到 width 变量中。
    else{break;}}
  }
  free(buf);screen_w=width;screen_h=height;
  if(*w==0&&*h==0){*w=screen_w;*h=screen_h;}//如果*w和*h都为0的话就赋值
  canvas_w = *w; canvas_h = *h;
  //保持画布在屏幕中央
  canvas_x=(screen_w - canvas_w) / 2;
  canvas_y=(screen_h - canvas_h) / 2;
  printf("screen_w:%d, screen_h:%d\n",screen_w,screen_h);
  printf("canvas_w:%d, canvas_h:%d\n",canvas_w,canvas_h);
  printf("canvas_x:%d, canvas_y:%d\n",canvas_x,canvas_y);
}

// 向画布`(x, y)`坐标处绘制`w*h`的矩形图像, 并将该绘制区域同步到屏幕上
// 图像像素按行优先方式存储在`pixels`中, 每个像素用32位整数以`00RRGGBB`的方式描述颜色
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
//NDL_DrawRect()的功能和PA2中介绍的绘图接口是非常类似的. 但为了实现它, NDL还需要知道屏幕大小的信息. 
// Nanos-lite和Navy约定, 屏幕大小的信息通过/proc/dispinfo文件来获得, 它需要支持读操作. 
// navy-apps/README.md中对这个文件内容的格式进行了约定, 你需要阅读它. 至于具体的屏幕大小, 你需要通过IOE的相应API来获取.
  int fd = open("/dev/fb", 0, 0);
  for (int i = 0; i < h && y + i < canvas_h; i++) { //i<h&&y+i<canvas_h是为了保证即使有部分显示不出来也好比越界访问出错好
    int now_line_in_buf = y + canvas_y + i;//确认现在在buf当中第几行
    int now_column_in_buf = x + canvas_x;//确认现在在buf当中的第几列
    lseek(fd, (now_column_in_buf* screen_w + now_column_in_buf) * sizeof(uint32_t), SEEK_SET);
    write(fd, pixels + i * w,  (w < canvas_w - x ? w : canvas_w - x) *sizeof(uint32_t));//倘若w大于画布宽度减去当前x的话宁愿少贴一点图也不要访问越界
    printf("x:%u\ny:%u\nw:%u\nh:%u\nfd:%u\nbuf:%u\nlen:%u\n",x,y,w,h,fd,pixels + i * w,(w < canvas_w - x ? w : canvas_w - x) *sizeof(uint32_t));
  }
  assert(close(fd) == 0);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  // 首先明确一下gettimeofday系统调用相关的ioe无论是使用clock_gettime还是gettimeofday（linux提供的版本，不是我们自己的），
  // 返回的时间都不是从0开始，前者是宿主机器开机到现在经过的时间，后者是从1970到现在经过的时间。
  // 所以我们还得保存一个NDL初始化时的时间，以便实现自从SDL库初始化到现在经过的时间的功能。

  //int SDLCALL SDL_Init(Uint32 flags)
  // 其中，flags可以取下列值：
  // SDL_INIT_TIMER：定时器
  // SDL_INIT_AUDIO：音频
  // SDL_INIT_VIDEO：视频
  // SDL_INIT_JOYSTICK：摇杆
  // SDL_INIT_HAPTIC：触摸屏
  // SDL_INIT_GAMECONTROLLER：游戏控制器
  // SDL_INIT_EVENTS：事件
  // SDL_INIT_NOPARACHUTE：不捕获关键信号（这个不理解）
  // SDL_INIT_EVERYTHING：包含上述所有选项

  //这里之后可能需要进行一些改动  但是还没有想好  因此先不管了
  

  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
