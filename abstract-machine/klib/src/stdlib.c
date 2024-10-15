#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#include <limits.h>
#include <stddef.h>

#define HEAP_SIZE (heap.end-heap.start)

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
  // 在native本地环境中，C运行时初始化时会调用malloc()。
  // 因此，不要在这里调用panic()，否则会导致死循环：
  // panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  // panic("Not implemented");
#endif
  // #if defined(__NATIVE_USE_KLIB__)
  // panic("Not implemented");
  // The  malloc(), calloc(), realloc(), and reallocarray() functions return
  //  a pointer to the allocated memory, which is suitably  aligned  for  any
  //  type  that fits into the requested size or less.  On error, these func‐
  //  tions return NULL and set errno.   Attempting  to  allocate  more  than
  //  PTRDIFF_MAX bytes is considered an error, as an object that large could
  //  cause later pointer subtraction to overflow.
  // 在C语言标准库中，malloc() 函数用于动态分配内存。根据手册页 man 3 malloc 的描述，malloc() 分配的内存块的指针需要满足一定的对齐要求，
  // 以确保它可以被赋给任何类型的指针，并且可以用来访问该类型的数据对象。具体来说，返回的指针必须适合对齐任何类型的数据对象，
  // 这意味着它通常需要符合以下对齐要求：
  // 在32位系统上，malloc() 返回的地址通常是8字节对齐的。
  // 在64位系统上，返回的地址可能是8字节或16字节对齐的，这取决于系统的架构和编译器的实现。
  // malloc() 函数分配的内存块的指针必须对齐到 size_t 类型的大小，这是因为 malloc() 的参数 size 是 size_t 类型，
  // 而 size_t 的大小通常与平台的指针大小相同。在大多数平台上，size_t 是一个无符号整数类型，其大小等于一个指针的大小
  //（例如，在32位系统上通常是4字节，在64位系统上通常是8字节）。
  // 此外，malloc() 分配的内存块的指针还必须对齐到足够大的边界，以容纳最大的基本数据类型（如 long double），
  // 这是为了确保返回的内存块可以用于任何类型的数据对象。在某些平台上，这可能意味着对齐到16字节的边界。
  // 总结来说，malloc() 返回的指针必须对齐到足够大的边界，以容纳任何基本数据类型，并且在大多数情况下，这意味着对齐到8字节或16字节的边界。
  // 这种对齐要求确保了分配的内存可以安全地用于任何类型的数据对象，而不会出现对齐错误。
  // 假设heap.start是堆的起始地址，这个值通常是在链接脚本中定义的
  // extern char heap_start[];
  extern Area heap;
  // extern char _heap_start;
  static char *addr = NULL; // 上次分配内存的位置
  static char *heap_end = NULL; // 堆的结束地址
  // printf("调用了malloc函数\n");
  // 初始化堆的起始和结束地址
  if (heap_end == NULL) {
      addr = heap.start;
      heap_end = heap.end;
      // printf("heap.start值为:%u\n",heap.start);
  }
  // if(addr==0) { addr=heap.start;printf("heap.start值为:%d\n", (char *)heap.start);}
  // 确保size是按对齐要求对齐的
  size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);//和 ~(0000000000011)进行与操作  和4字节进行对齐
  if (((void*)addr + size) > heap.start + HEAP_SIZE) { // HEAP_SIZE需要定义堆的大小
      // 堆溢出，返回NULL
      printf("堆溢出\n");
      return NULL;
  }
  void *malloced = addr;
  addr += size; // 更新addr为下一次分配的起始地址
  return malloced;

#endif
  return NULL;
}

void free(void *ptr) {
}


