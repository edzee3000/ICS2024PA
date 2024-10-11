#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdlib.h>
// #include <regex.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
// #if !defined(__ISA_NATIVE__) || !defined(__NATIVE_USE_KLIB__)
#define MAX_LEN_INPUT (uint32_t)(65536)

// static struct rule {
//   const char *regex;
//   int token_type;
// } rules[] = {

//   /* TODO: Add more rules.添加更多的规则
//    * Pay attention to the precedence level of different rules.注意不同规则之间的优先级关系
//    */
//   {}
// };

int printf(const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  char out[256];
  int len=vsprintf(out,fmt,args);
  for(int i=0;i<len;i++)
    putch(out[i]);
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  //panic("Not implemented");
  uint32_t n=MAX_LEN_INPUT;
  return vsnprintf(out,n,fmt,ap);
}



int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  return vsprintf(out,fmt,args);
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  //panic("Not implemented");
  const char *p=fmt;
  size_t i=0;
  size_t j=0;
  while(p[i]!='\0' && j<n)
  {
    if(p[i]=='%')
    {
      i++;
      if(p[i]=='d'||p[i]=='i')
      {
        int num=va_arg(ap,int);
        char tem[128];int k=0;
        while(num!=0){int t=num%10;num/=10;tem[k]='0'+t; k++;} tem[k]='\0';
        for(int l=k-1;l>=0;l--){out[j]=tem[l]; j++;       if(j>=n)break; } 
      }
      else if( p[i]=='c'){//输入一个字符
        char c=va_arg(ap,int);
        out[j]=c; j++;            
        if(j>=n)break;
      }
      else if( p[i]=='s')//输入一个字符串
      { 
        const char *ch=va_arg(ap,const char*);
        while(*ch!='\0'){out[j]=*ch;ch++;j++;     if(j>=n)break;}
      }
      else if( p[i]<='9'&&p[i]>='0'  && p[i+1]=='2'  &&p[i+2]=='d' ){i+=2;  int num=va_arg(ap,int);char tem[10];int k=0;while(num!=0){int t=num%10;num/=10;tem[k]='0'+t; k++;} tem[k]='\0';for(int l=k-1;l>=0;l--){out[j]=tem[l]; j++;       if(j>=n)break; }  }



    }
    else{out[j]=p[i];j++;}
    i++;
    if(j>=n)break;
  }
  out[j]='\0';
  //在 stdio.h 中，sprintf 与 vsprintf 函数的返回值是成功写入缓冲区的字符数量。这个返回值不包括终止的空字符（\0）。如果发生输出错误，函数会返回一个负数。
  return j;
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  return vsnprintf(out,n,fmt,args);
}


#endif
