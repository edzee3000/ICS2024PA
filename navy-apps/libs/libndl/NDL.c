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
static int screen_w = 0, screen_h = 0;

enum{SDL_INIT_TIMER,SDL_INIT_AUDIO,SDL_INIT_VIDEO,
SDL_INIT_JOYSTICK,SDL_INIT_HAPTIC,SDL_INIT_GAMECONTROLLER,
SDL_INIT_EVENTS,SDL_INIT_NOPARACHUTE,SDL_INIT_EVERYTHING};

uint32_t NDL_GetTicks() {
  //SDL_GetTicks这个函数功能返回自从SDL库初始化到现在经过的毫秒值
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  return 0;
}

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
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
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

  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
