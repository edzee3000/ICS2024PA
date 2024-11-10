#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

//为了在Navy中运行Flappy Bird, 你还需要实现另一个库SDL_image中的一个API: IMG_Load(). 
// 这个库是基于stb项目中的图像解码库来实现的, 用于把解码后的像素封装成SDL的Surface结构, 这样应用程序就可以很容易地在屏幕上显示图片了. 
// 上述API接受一个图片文件的路径, 然后把图片的像素信息封装成SDL的Surface结构并返回.
// 这个API的一种实现方式如下:
// 1. 用libc中的文件操作打开文件, 并获取文件大小size
// 2. 申请一段大小为size的内存区间buf
// 3. 将整个文件读取到buf中
// 4. 将buf和size作为参数, 调用STBIMG_LoadFromMemory(), 它会返回一个SDL_Surface结构的指针
// 5. 关闭文件, 释放申请的内存
// 6. 返回SDL_Surface结构指针
SDL_Surface* IMG_Load(const char *filename) {
  //用libc中的文件操作打开文件, 并获取文件大小size
  FILE *file = fopen(filename, "rb");
  if(file == NULL) {perror("打开文件失败\n");return NULL;}
  fseek(file, 0, SEEK_END);//移动到文件末尾
  uint32_t size = ftell(file);//调用ftell函数返回文件指针相对于文件开头的当前位置  在调用fseek之后文件指针位于文件的末尾 所以ftell返回的是从文件开头到文件末尾的字节数即文件的大小。
  rewind(file);  //然后调用rewind函数将文件指针重新定位到文件的开始位置  这通常在获取文件大小之后执行，以便后续的文件操作可以从文件的开头开始
  // 申请一段大小为size的内存区间buf
  unsigned char *buf = (unsigned char *)malloc(size*sizeof(unsigned char));
  if(buf == NULL) {perror("动态内存分配失败\n");fclose(file);return NULL;}
  //将整个文件读取到buf中
  if(fread(buf, 1, size, file)!=1){perror("读取文件失败\n");free(buf);fclose(file);return NULL;}
  //将buf和size作为参数, 调用STBIMG_LoadFromMemory(), 它会返回一个SDL_Surface结构的指针
  SDL_Surface *surface = STBIMG_LoadFromMemory(buf, size);
  if(surface == NULL) {fprintf(stderr, "从内存中读取图片失败\n");free(buf);fclose(file);return NULL;}
  //关闭文件, 释放申请的内存
  fclose(file);free(buf);
  //返回SDL_Surface结构指针
  printf("insert\n");
  return surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
