#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  if(s==NULL){printf("传入了空指针，通常为UB未定义行为，此处仅作测试，返回0\n");return 0;}
  size_t len=0;
  while(s[len]!='\0')len++;
  return len;
  //panic("Not implemented");
  //根据strlen1的manual显示：注意长度是不算上'\0'的！！！！！！！！！！！！！！！！！！！
  // The strlen() function calculates the length of the string pointed to by
  //     s, excluding the terminating null byte ('\0').
}

char *strcpy(char *dst, const char *src) {
  //下面是我的实现  感觉没有strcpy源代码写的好，因此附加了一段头文件中的代码
  char *ptr1 = dst;
  const char *ptr2=src;
  size_t i=0;
  while(ptr2[i]!='\0')
  {
    ptr1[i]=ptr2[i];
    i++;
  }
  ptr1[i]='\0';
  return dst;
  //panic("Not implemented");
  //These functions copy the string pointed to by src, into a string
  //            at  the buffer pointed to by dst.  The programmer is responsible
  //            for allocating a  destination  buffer  large  enough,  that  is,
  //            strlen(src)  + 1.  For the difference between the two functions,
  //            see RETURN VALUE.
  
  // assert((dst != NULL) || (src != NULL));   //[2]
  // char *address = dst;					 //[3]
  // while((*dst++ = *src++)!='\0');		 //[4]
  // return address;
}
 
char *strncpy(char *dst, const char *src, size_t n) {
  size_t i=0;
  size_t len_src=strlen(src)+1;//src的长度是算上'\0'的
  for(;i<n && i < len_src ; i++)dst[i]=src[i];
  return dst;
  //panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  //先寻找dst尾部（不包含'\0'）
  if(dst==NULL||src==NULL)return NULL;
  char *begin= dst+strlen(dst);
  strcpy(begin , src);
  return dst;
  //panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  if(s1==NULL&&s2==NULL){printf("注意此时你传进来了2个NULL空指针\n");return 0;}
  else if(s1==NULL){printf("注意此时你传进来的第一个参数是个NULL空指针\n");return -1;}
  else if(s2==NULL){printf("注意此时你传进来的第二个参数是个NULL空指针\n");return 1;}
  else if(strlen(s1)==1&&strlen(s2)==1){return 0;}
  //panic("Not implemented");
  size_t len_s1=strlen(s1)+1;//算上'\0'的长度
  size_t len_s2=strlen(s2)+1;
  size_t i=0;
  for(;i < len_s1 && i < len_s2;i++)
  {
    if(s1[i]>s2[i]) return 1;
    else if (s1[i]<s2[i]) return -1;
  }
  if(len_s1==len_s2) return 0;
  else if(i==len_s1) return -1;
  else if(i==len_s2) return 1;
  else return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  //panic("Not implemented");
  if(s1==NULL&&s2==NULL){printf("注意此时你传进来了2个NULL空指针\n");return 0;}
  else if(s1==NULL){printf("注意此时你传进来的第一个参数是个NULL空指针\n");return -1;}
  else if(s2==NULL){printf("注意此时你传进来的第二个参数是个NULL空指针\n");return 1;}
  else if(n==0)return 0;
  size_t len_s1=strlen(s1);//不算上'\0'的长度
  size_t len_s2=strlen(s2);
  size_t i=0;
  for(;i<len_s1 && i<len_s2 && i<n;i++)
  {
    if(s1[i]>s2[i])return 1;
    else if (s1[i]<s2[i]) return -1;
    else continue;
  }
  if(i==n) return 0;
  if(len_s1==len_s2) return 0;
  else if(i==len_s1) return -1;
  else if(i==len_s2) return 1;
  else return 0;

  /*以下是glibc当中的源代码  这里列出来仅作对比 
  Compare no more than N characters of S1 and S2,
   returning less than, equal to or greater than zero
   if S1 is lexicographically less than, equal to or
   greater than S2.  */

  // unsigned char c1 = '\0';
  // unsigned char c2 = '\0';
 
  // if (n >= 4)
  // {
  //   size_t n4 = n >> 2;
  //   do
	//   {
  //     c1 = (unsigned char) *s1++;
  //     c2 = (unsigned char) *s2++;
  //     if (c1 == '\0' || c1 != c2)
  //       return c1 - c2;
  //     c1 = (unsigned char) *s1++;
  //     c2 = (unsigned char) *s2++;
  //     if (c1 == '\0' || c1 != c2)
  //       return c1 - c2;
  //     c1 = (unsigned char) *s1++;
  //     c2 = (unsigned char) *s2++;
  //     if (c1 == '\0' || c1 != c2)
  //       return c1 - c2;
  //     c1 = (unsigned char) *s1++;
  //     c2 = (unsigned char) *s2++;
  //     if (c1 == '\0' || c1 != c2)
  //       return c1 - c2;
	//   } while (--n4 > 0);
  //     n &= 3;
  //   }
 
  // while (n > 0)
  //   {
  //     c1 = (unsigned char) *s1++;
  //     c2 = (unsigned char) *s2++;
  //     if (c1 == '\0' || c1 != c2)
	// return c1 - c2;
  //     n--;
  //   }
 
  // return c1 - c2;

}

void *memset(void *s, int c, size_t n) {
  uint8_t *ptr = (uint8_t *)s;
  size_t i=0;
  for (; i < n; i++) ptr[i] = (uint8_t)c;
  return (void *)s;
  //panic("Not implemented");
  //The  memset()  function  fills  the  first  n  bytes of the memory area
  //     pointed to by s with the constant byte c.
}

void *memmove(void *dst, const void *src, size_t n) {
  if(dst==NULL&&src==NULL){printf("注意此时你传进来了2个NULL空指针\n");return dst;}
  else if(dst==NULL){printf("注意此时你传进来的第一个参数是个NULL空指针\n");return dst;}
  else if(src==NULL){printf("注意此时你传进来的第二个参数是个NULL空指针\n");return dst;}
  else if(n==0){*(char*)dst='\0';return dst;}
  //panic("Not implemented");
  // uint8_t *temp=(uint8_t *)malloc(n);
  uint8_t temp[1024];
  uint8_t *s1=(uint8_t*)dst;
  uint8_t *s2=(uint8_t*)src;
  size_t i;
  for(i=0;i<n;i++) temp[i]=s2[i];
  for(i=0;i<n;i++) s1[i]=temp[i];
  return (void *)dst;

  // void *ret=dst;
  // //这里来判别dest和src是否是指向同一字符串中不同位置，
  // //如果是指向同一字符串，但是dest在src前面，则可以从前往后逐个赋值
  // //如果是指向同一字符串，但是dest在src后面，且dest>=src+count,那么仍然从前往后赋值
  // if(dst<=src||dst>=src+n)
  // {
  //     while(n--)
  //         *(char*)dst++=*(char*)src++;
  // }
  // //如果是指向同一字符串，但是dest在src后面，且dest<=src+count,那么从后往前赋值
  // else
  // {
  //     dst+=n-1;
  //     src+=n-1;
  //     while(n--)
  //         *(char*)dst--=*(char*)src--;
  // }
  // return ret;


  //memmove() 函数是 C 语言标准库中的一个函数，用于拷贝内存区域。与 memcpy() 不同的是，memmove() 可以安全地处理源内存区域和目标内存区域重叠的情况
  //memmove() 函数会从 src 指向的内存区域拷贝 n 个字节到 dest 指向的内存区域。
  //如果源内存和目标内存有重叠，memmove() 会正确处理，因为它总是从源内存的开始位置向目标内存的开始位置拷贝数据，从而避免了在拷贝过程中覆盖源数据的问题
}

void *memcpy(void *out, const void *in, size_t n) {
  //以下是我自己写的代码  感觉不如源代码写的好  直接贴源代码了
  // uint8_t * dst=(uint8_t *)out;
  // const uint8_t *src =(const uint8_t*)in;
  // size_t i=0;
  // for(;i<n;i++)
  //   dst[i]=src[i];
  // return out;

  //panic("Not implemented");

  void *ret = out;
	
	if(out <= in || (char *)out >= (char *)in + n){
		//没有内存重叠，从低地址开始复制
		while(n--){
			*(char *)out = *(char *)in;
			out = (char *)out + 1;
			in = (char *)in + 1;
		}
	}else{
		//有内存重叠，从高地址开始复制
    // printf("存在内存重叠\n");
		out = (char *)out + n - 1;
    in = (char *)in + n - 1;
		while(n--){
			*(char *)out = *(char *)in;
			out = (char *)out - 1;
			in = (char *)in - 1;
		}
	}
  return ret;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  if(s1==NULL&&s2==NULL){printf("注意此时你传进来了2个NULL空指针\n");return 0;}
  else if(s1==NULL){printf("注意此时你传进来的第一个参数是个NULL空指针\n");return -1;}
  else if(s2==NULL){printf("注意此时你传进来的第二个参数是个NULL空指针\n");return 1;}
  else if(n==0)return 0;
  unsigned char *ptr1=(unsigned char *)s1;
  unsigned char *ptr2=(unsigned char *)s2;
  size_t i=0;
  for(;i<n;i++)
  {
    if(ptr1[i]>ptr2[i])return 1;
    else if (ptr1[i]<ptr2[i]) return -1;
    else continue;
  }
  return 0;

  if(s1 == NULL || s2 == NULL){
		return 0;
	}

  //panic("Not implemented");
  //The  memcmp()  function compares the first n bytes (each interpreted as注意每一个都是被解释为unsigned char的
  //     unsigned char) of the memory areas s1 and s2.
}

#endif



//注意在我写的strcpy函数是没有考虑到区间重叠的情况的  但是在memcpy里面我是考虑到了区间重叠的情况的  因此这里暂时先埋了个坑  记得要回来补的！！！！！！！！
//在我自己写的memcpy当中是对区间重叠的情况考虑的，但是在标准库当中memcpy是没有对区间重叠的部分进行处理的
//而是在memmove函数当中进行了考虑处理的  （万一oj要测试的话那我就把我写的memcpy函数移植到memmove当中去）

