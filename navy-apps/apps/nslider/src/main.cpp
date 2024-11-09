#include <SDL.h>
#include <SDL_bmp.h>
#include <stdio.h>
#include <assert.h>

#define W 400
#define H 300

// USAGE:
//   j/down - page down
//   k/up - page up
//   gg - first page

// number of slides
// const int N = 10;
const int N = 61;
// slides path pattern (starts from 0)
const char *path = "/share/slides/slides-%d.bmp";

static SDL_Surface *screen = NULL;
static int cur = 0;

void render() {
  char fname[256];
  sprintf(fname, path, cur);
  SDL_Surface *slide = SDL_LoadBMP(fname);
  assert(slide);
  SDL_BlitSurface(slide, NULL, screen, NULL); //这里srcrect和dstrect指针都是NULL说明是按照屏幕原封不动移动过来的
  SDL_UpdateRect(screen, 0, 0, 0, 0);
  SDL_FreeSurface(slide);
}

void prev(int rep) {
  if (rep == 0) rep = 1;
  cur -= rep;
  if (cur < 0) cur = 0;
  render();
}

void next(int rep) {
  if (rep == 0) rep = 1;
  cur += rep;
  if (cur >= N) cur = N - 1;
  render();
}

int main() {
  SDL_Init(0);
  screen = SDL_SetVideoMode(W, H, 32, SDL_HWSURFACE);

  int rep = 0, g = 0;

  render();

  while (1) {
    SDL_Event e;
    SDL_WaitEvent(&e);//while循环等待事件e是否被按下

    if (e.type == SDL_KEYDOWN) {
      switch(e.key.keysym.sym) {//e.key.keysym.sym存储的是键对应的值
        case SDLK_0: rep = rep * 10 + 0; break;  //对应刚好是0这个键
        case SDLK_1: rep = rep * 10 + 1; break;  //按下0-9之间的键表示记录要跳转到多少页  比如 1 2 3 就表示往后/往前跳123页
        case SDLK_2: rep = rep * 10 + 2; break;
        case SDLK_3: rep = rep * 10 + 3; break;
        case SDLK_4: rep = rep * 10 + 4; break;
        case SDLK_5: rep = rep * 10 + 5; break;
        case SDLK_6: rep = rep * 10 + 6; break;
        case SDLK_7: rep = rep * 10 + 7; break;
        case SDLK_8: rep = rep * 10 + 8; break;
        case SDLK_9: rep = rep * 10 + 9; break;
        case SDLK_J:
        case SDLK_DOWN: next(rep); rep = 0; g = 0; break;  //按下J键或者DOWN键表示
        case SDLK_K:
        case SDLK_UP: prev(rep); rep = 0; g = 0; break;
        case SDLK_G:
          g ++;
          if (g > 1) {
            prev(100000);//表示回到最开始因为100000是个很大的数字
            rep = 0; g = 0;
          }
          break;
      }
    }
  }

  return 0;
}
