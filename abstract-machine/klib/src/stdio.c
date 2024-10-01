#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  const char *p=fmt;
  size_t i=0;
  size_t j=0;
  while(p[i]!='\0')
  {
    if(p[i]=='%')
    {
      i++;
      switch (p[i])
      {
      case 'd': case 'i': case 'c'://输入一个整数
        int num=va_arg(args,int);
        char tem[128];int k=0;
        while(num!=0){int t=num%10;num/=10;tem[k]='0'+t; k++;} tem[k]='\0';
        for(int l=k-1;l>=0;l--){out[j]=tem[l]; j++;} 
        break;
      case 's':
        const char *ch=va_arg(args,const char*);
        while(*ch!='\0'){out[j]=*ch;ch++;j++;}
        break;
      default:
        break;
      }
    }
    else{out[j]=p[i];j++;}
    i++;
  }
  //在 stdio.h 中，sprintf 函数的返回值是成功写入缓冲区的字符数量。这个返回值不包括终止的空字符（\0）。如果发生输出错误，函数会返回一个负数。
  return j;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
