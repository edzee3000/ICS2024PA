#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
char *out;
int printf(const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  // char *out=(char *)malloc(32);//假设一行最多只有256个字符
  int len=vsprintf(out,fmt,args);
  for(int i=0;i<len;i++)
    putch(out[i]);
  free(out);
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  //panic("Not implemented");
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
        int num=va_arg(ap,int);
        char tem[128];int k=0;
        while(num!=0){int t=num%10;num/=10;tem[k]='0'+t; k++;} tem[k]='\0';
        for(int l=k-1;l>=0;l--){out[j]=tem[l]; j++;} 
        break;
      case 's'://输入一个字符串
        const char *ch=va_arg(ap,const char*);
        while(*ch!='\0'){out[j]=*ch;ch++;j++;}
        break;
      default:break;
      }
    }
    else{out[j]=p[i];j++;}
    i++;
  }
  out[j]='\0';
  //在 stdio.h 中，sprintf 与 vsprintf 函数的返回值是成功写入缓冲区的字符数量。这个返回值不包括终止的空字符（\0）。如果发生输出错误，函数会返回一个负数。
  return j;
}



int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  return vsprintf(out,fmt,args);
}




int snprintf(char *out, size_t n, const char *fmt, ...) {
  //panic("Not implemented");
  return 0;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  //panic("Not implemented");
  return 0;
}

#endif
