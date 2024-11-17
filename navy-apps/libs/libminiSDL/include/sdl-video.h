#ifndef __SDL_VIDEO_H__
#define __SDL_VIDEO_H__

#define SDL_HWSURFACE 0x1
#define SDL_PHYSPAL 0x2
#define SDL_LOGPAL 0x4
#define SDL_SWSURFACE  0x8
#define SDL_PREALLOC  0x10
#define SDL_FULLSCREEN 0x20
#define SDL_RESIZABLE  0x40

#define DEFAULT_RMASK 0x00ff0000
#define DEFAULT_GMASK 0x0000ff00
#define DEFAULT_BMASK 0x000000ff
#define DEFAULT_AMASK 0xff000000

typedef struct {
	int16_t x, y;
	uint16_t w, h;
} SDL_Rect;

typedef union {
  struct {
    uint8_t r, g, b, a;
  };
  uint32_t val;
} SDL_Color;

typedef struct {
	int ncolors;//指出调色板里面有多少颜色
	SDL_Color *colors;//颜色数组
} SDL_Palette;//调色板，也是一个结构体类型
// 调色板有什么用呢？这里还得说一下位图的存储方式。
// 需要注意的是：SDL_Surface本身只能存储位图数据（因为其官方只给了SDL_LoadBMP()函数来加载位图，而没有函数去加载其他格式的图片），
// 虽然有SDL_Image库，但那是第三方的不算在讨论范围内，所以我们首先得搞清楚调色板在位图中的用途。
// 调色板中记录了这个图片中所有要用到的颜色（对于256色位图，就会记录256色），这也就是ncolors属性的作用。
// 然后会将所有属性以RGBxxx(例子里面是RGB888)的方式存储在colors属性中。 然后原本的像素就不再以RGB888方式存储了，
// 其会存储一个索引，这个索引指向colors属性中的颜色



typedef struct {
	SDL_Palette *palette;//调色板（如果没有是NULL）
	uint8_t BitsPerPixel;//每一个像素使用多少个Bit存储
	uint8_t BytesPerPixel; //每个像素使用多少个Byte存储
	uint8_t Rloss, Gloss, Bloss, Aloss; //R G B A分量的掩码
	uint8_t Rshift, Gshift, Bshift, Ashift;
	uint32_t Rmask, Gmask, Bmask, Amask;
} SDL_PixelFormat;

typedef struct {
	uint32_t flags;
	SDL_PixelFormat *format; //存储着和像素有关的格式                read-only
	int w, h;				//图像宽度和高度                   read-only
	uint16_t pitch;   //pixels中一行有多少个像素（以Bytes计） read-only
	uint8_t *pixels; //实际的像素数据                        read-write
} SDL_Surface;

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags);
void SDL_FreeSurface(SDL_Surface *s);
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color);
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h);
void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors);
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags);
uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int SDL_LockSurface(SDL_Surface *s);
void SDL_UnlockSurface(SDL_Surface *s);

#endif
