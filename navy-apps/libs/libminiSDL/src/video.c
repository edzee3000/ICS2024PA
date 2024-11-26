#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//SDL的绘图模块引入了一个Surface的概念, 它可以看成一张具有多种属性的画布, 具体可以通过RTFM查阅Surface结构体中的成员含义. 

#define WINDOW_W 800  //定义当前窗口宽度和高度  但是这个是在其他地方哪里有的？？？？？？？？好奇怪……
#define WINDOW_H 600

//为了在Navy中运行仙剑奇侠传, 你还需要对miniSDL中绘图相关的API进行功能的增强. 具体地, 作为一款上世纪90年代的游戏, 
// 绘图的时候每个像素都是用8位来表示, 而不是目前普遍使用的32位00RRGGBB. 而这8位也并不是真正的颜色, 而
// 是一个叫"调色板"(palette)的数组的下标索引, 调色板中存放的才是32位的颜色. 用代码的方式来表达, 就是:
// // 现在像素阵列中直接存放32位的颜色信息
// uint32_t color_xy = pixels[x][y];

// // 仙剑奇侠传中的像素阵列存放的是8位的调色板下标,
// // 用这个下标在调色板中进行索引, 得到的才是32位的颜色信息
// uint32_t pal_color_xy = palette[pixels[x][y]];
static uint32_t trans_color_from_8_to_32(SDL_Color *c)
{return (c->a << 24) | (c->r << 16) | (c->g << 8) | c->b;//将8位color转换成32位color
}

//SDL_BlitSurface(): 将一张画布中的指定矩形区域复制到另一张画布的指定位置  在NJU Slider中要实现的
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  SDL_Rect src_rect = { .x = 0, .y = 0, .w = src->w, .h = src->h };//初始化两个空的矩形分别为src和dst的大小
  SDL_Rect dst_rect = { .x = 0, .y = 0, .w = dst->w, .h = dst->h };
  if(srcrect)src_rect=*srcrect;
  if(dstrect)dst_rect=*dstrect;//这里一开始写错了 将*dstrect赋值给了srcrect  导致在menu菜单apps当中上下翻页的过程中一直显示是fillrect的白色  最后在menu.cpp当中RTFSC才修改过来
  uint32_t Pixel_Bit=dst->format->BitsPerPixel;
  uint8_t* Src_Pixel_Data = src->pixels;
  uint8_t* Dst_Pixel_Data = dst->pixels;
  //将对应的矩形区域复制到dst的矩形区域  但是在这里我并没有考虑到边界情况  暂时先不管
  for (uint32_t i = 0; i < src_rect.h; i ++){for (uint32_t j = 0; j < src_rect.w; j ++){
    uint32_t pixel_dst_index=(i + dst_rect.y) * dst->w + j + dst_rect.x;
    uint32_t pixel_src_index=(i + src_rect.y) * src->w + j + src_rect.x;
    switch (Pixel_Bit){
      case 8: dst->pixels[pixel_dst_index]=src->pixels[pixel_src_index];break;
      case 32: ((uint32_t*)dst->pixels)[pixel_dst_index]=((uint32_t*)src->pixels)[pixel_src_index];break;
      default:break;}
  }}
  
}


//在MENU开机菜单中要实现的
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {//用color来填充整个矩形
  SDL_Rect dst_rect = { .x = 0, .y = 0, .w = dst->w, .h = dst->h };//初始化矩形为整个目标屏幕大小
  if (dstrect) {dst_rect = *dstrect;}
  uint32_t Pixel_Bit=dst->format->BitsPerPixel;
  for (int i = 0; i < dst_rect.h; i ++) {for (int j = 0; j < dst_rect.w; j ++) 
  {
    uint32_t pixel_index=(i + dst_rect.y) * dst->w + j + dst_rect.x;
    switch (Pixel_Bit)
      {case 32: ((uint32_t *)dst->pixels)[pixel_index]=color;break;
       case 8: (dst->pixels)[pixel_index]=color;break;
      default:break;}
  }}
  //开机菜单是另一个行为比较简单的程序, 它会展示一个菜单, 用户可以选择运行哪一个程序. 为了运行它, 
  // 你还需要在miniSDL中实现SDL_FillRect(), 它用于往画布的指定矩形区域中填充指定的颜色.
  //正确实现上述API后, 你将会看到一个可以翻页的开机菜单. 但你尝试选择菜单项的时候将会出现错误, 
  // 这是因为开机菜单的运行还需要一些系统调用的支持. 我们会在下文进行介绍, 目前通过开机菜单来测试miniSDL即可.
}

//SDL_UpdateRect(): 将画布中的指定矩形区域同步到屏幕上  在NJU Slider中要实现的
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  if(w==0||s==0){w=s->w;h=s->h;}
  uint32_t local_pixels[WINDOW_W * WINDOW_H];
  uint32_t Pixel_Bit=s->format->BitsPerPixel;//根据每个像素所占bit位数不同需要进行分类讨论
  SDL_Color* colors = s->format->palette->colors; //s->format->palette->colors是一个SDL_Color类型的数组，以8位颜色为下标时可以获得其对应的SDL_Color，结构体包含rgba四个8位数字，再写一个函数将4个8位数字转化为一个32位数。
  if(Pixel_Bit==32)
  {NDL_DrawRect((uint32_t*)(s->pixels),x,y,w,h);return ;}
  if (Pixel_Bit==8)
  {
    for (int i = 0; i < h; i ++) {for (int j = 0; j < w; j ++) //找出pixel对应调色盘索引对应的值
    {uint8_t pixel_index= s->pixels[(i+y)*s->w+j+x]; //注意这里的pixel_index是
    // local_pixels[i * w + j]=colors[pixel_index].val;
    //#############################################################################
    local_pixels[i * w + j]=trans_color_from_8_to_32(&colors[pixel_index]);
    //#############################################################################
    //SDL_UpdateRect()在最后调用NDL_DrawRect()，传入的参数pixels必须是32位格式的，不然会显示错误颜色。
    //因此需要现将8位颜色转为32位的，再填入pixels
    }}
    NDL_DrawRect(local_pixels, x, y, w, h);
    return ;
  }
  
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
  uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8) {//对于位数为8位的情况
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
}
