#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len=1;
  while(s[len-1]!='\0')len++;
  return len;
  //panic("Not implemented");
  //根据strlen1的manual显示：注意长度是需要算上'\0'的
  // The strlen() function calculates the length of the string pointed to by
  //     s, excluding the terminating null byte ('\0').
}

char *strcpy(char *dst, const char *src) {
  char *ptr1=dst;
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
 
}
 
char *strncpy(char *dst, const char *src, size_t n) {
  size_t i=0;
  size_t len_src=strlen(src);
  for(;i<n && i < len_src-1 ; i++)dst[i]=src[i];
  return dst;
  //panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  //先寻找dst尾部（不包含'\0'）
  char *begin= dst+strlen(dst)-1;
  strcpy(begin , src);
  return dst;
  //panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  //panic("Not implemented");
  size_t len_s1=strlen(s1);//算上'\0'的长度
  size_t len_s2=strlen(s2);
  size_t i=0;
  for(;i < len_s1-1 && i < len_s2-1;i++)
  {
    if(s1[i]>s2[i]) return 1;
    else if (s1[i]<s2[i]) return -1;
  }
  if(len_s1==len_s2) return 0;
  else if(i==len_s1-1) return -1;
  else if(i==len_s2-1) return 1;
  else return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  //panic("Not implemented");
  size_t len_s1=strlen(s1);//算上'\0'的长度
  size_t len_s2=strlen(s2);
  size_t i=0;
  for(;i<len_s1-1 && i<len_s2-1 && i<n;i++)
  {
    if(s1[i]>s2[i])return 1;
    else if (s1[i]<s2[i]) return -1;
    else continue;
  }
  if(i==n) return 0;
  if(len_s1==len_s2) return 0;
  else if(i==len_s1-1) return -1;
  else if(i==len_s2-1) return 1;
  else return 0;
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
  //panic("Not implemented");
  uint8_t *temp=(uint8_t *)malloc(n);
  uint8_t *s1=(uint8_t*)dst;
  uint8_t *s2=(uint8_t*)src;
  size_t i=0;
  for(;i<n;i++) temp[i]=s2[i];
  for(;i<n;i++) s1[i]=temp[i];
  return (void *)dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  uint8_t * dst=(uint8_t *)out;
  const uint8_t *src =(const uint8_t*)in;
  size_t i=0;
  for(;i<n;i++)
    dst[i]=src[i];
  return out;
  //panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
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
  //panic("Not implemented");
  //The  memcmp()  function compares the first n bytes (each interpreted as注意每一个都是被解释为unsigned char的
  //     unsigned char) of the memory areas s1 and s2.
}

#endif
