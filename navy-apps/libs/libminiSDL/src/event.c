#include <NDL.h>
#include <SDL.h>

#include <string.h>
#include "assert.h"

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
//定义一共有多少个按键  给每一个按键一个对应的状态
#define KEYNUM sizeof(keyname) / sizeof(keyname[0])
uint8_t key_status[KEYNUM] = {0};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

//轮询当前挂起的事件
int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf)))//调用NDL_PollEvent的函数接口
  {if(buf[0]=='k')
  { 
    char key_type;char key_buf[32]; //32个字节应该足足够了
    assert(sscanf(buf, "k%c %s\n", &key_type, key_buf) == 2);//从buf当中读取key_type以及key的名字keyname  对于key_type而言d表示down u表示up
    switch (key_type)
    {case 'd':ev->type=SDL_KEYDOWN; break;
    case 'u':ev->type=SDL_KEYUP; break;
    default:printf("key_type出错\n");assert(0);break;}
    for(int i=0;i<KEYNUM;i++){if(strcmp(keyname[i], key_buf) == 0){
      ev->key.keysym.sym=i; key_status[i] = !ev->type;//注意SDL_KEYDOWN为1 SDL_KEYUP为0   但是key_status表示的是0为up 1为down
      return 1;}}
  }}
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  while (true) { if (SDL_PollEvent(event)) return 1;}//一直等待Event事件直到有按键按下/弹起
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return key_status;//这里是key_status还是key_status+*numkeys？？？########################################
}
