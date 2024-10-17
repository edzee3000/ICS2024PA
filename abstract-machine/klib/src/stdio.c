#include <am.h>
#include <klib.h>
#include <klib-macros.h>
// #include <stdarg.h>
// #include <stdlib.h>
#include <regex.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define MAX_LEN_INPUT (uint32_t)(1024)
static char NUM_CHAR[] = "0123456789ABCDEF";    // 为后面取余做铺垫
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
  // //panic("Not implemented");
  // const char *p=fmt;
  // size_t i=0;
  // size_t j=0;
  // while(p[i]!='\0' && j<n)
  // {
  //   if(p[i]=='%')
  //   {
  //     i++; 
  //     if(p[i]=='d'||p[i]=='i')
  //     {
  //       int num=va_arg(ap,int);
  //       char tem[128];int k=0;
  //       if(num==0){ out[j]='0';j++; if(j>=n)break;}   
  //       while(num!=0){int t=num%10;num/=10;tem[k]='0'+t; k++;} tem[k]='\0';
  //       for(int l=k-1;l>=0;l--){out[j]=tem[l]; j++;       if(j>=n)break; } 
  //     }
  //     else if( p[i]=='c'){//输入一个字符
  //       char c=va_arg(ap,int);
  //       out[j]=c; j++;            
  //       if(j>=n)break;
  //     }
  //     else if( p[i]=='s')//输入一个字符串
  //     { 
  //       const char *ch=va_arg(ap,const char*);
  //       while(*ch!='\0'){out[j]=*ch;ch++;j++;     if(j>=n)break;}
  //     }
  //     // else if( p[i]<='9'&&p[i]>='0'  && p[i+1]=='2'  &&p[i+2]=='d' ){i+=2;  int num=va_arg(ap,int);char tem[10];int k=0;if(num==0){ out[j]='0';j++; if(j>=n)break;}   while(num!=0){int t=num%10;num/=10;tem[k]='0'+t; k++;} tem[k]='\0';for(int l=k-1;l>=0;l--){out[j]=tem[l]; j++;       if(j>=n)break; }  }
  //   }
  //   else{out[j]=p[i];j++;}
  //   i++;
  //   if(j>=n)break;
  // }
  // out[j]='\0';
  // //在 stdio.h 中，sprintf 与 vsprintf 函数的返回值是成功写入缓冲区的字符数量。这个返回值不包括终止的空字符（\0）。如果发生输出错误，函数会返回一个负数。
  // return j;
  int len = 0;
  char buf[128];
  int buf_len = 0;
  while(*fmt != '\0' && len < n){
    switch(*fmt) {
      case '%':
        fmt++;
        // 检查百分号之后的字符
        switch(*fmt) {
          case 'd':
            int val = va_arg(ap, int);    // 将该参数转为int型
            if(val == 0) out[len++] = '0';
            if(val < 0) {
              out[len++] = '-';
              val = 0 - val;
            }
            for(buf_len = 0; val; val /= 10, buf_len++)
              buf[buf_len] = NUM_CHAR[val % 10];    //这里buf会是逆序的
            for(int i = buf_len - 1; i >= 0; i--)
              out[len++] = buf[i];
            break;
          case 'u':
            uint32_t uval = va_arg(ap, uint32_t);
            // 同%d, 只不过不用考虑负数
            if(uval == 0) out[len++] = '0';
            for(buf_len = 0; uval; uval /= 10, buf_len++)
              buf[buf_len] = NUM_CHAR[uval % 10];    //这里buf会是逆序的
            for(int i = buf_len - 1; i >= 0; i--)
              out[len++] = buf[i];
            break;
          case 'c':
            char c = (char)va_arg(ap, int);    //va_arg函数没有char这个参数
            out[len++] = c;
            break;
          case 's':
            char *s = va_arg(ap, char*);
            for(int i = 0; s[i] != '\0'; i++)
              out[len++] = s[i];
            break;
          case 'x':
            out[len++] = '0'; out[len++] = 'x';
            uint32_t address = va_arg(ap, uint32_t);
            for(buf_len = 0; address; address /= 16, buf_len++)
              buf[buf_len] = NUM_CHAR[address % 16];
            for(int i = buf_len - 1; i >= 0; i--)
              out[len++] = buf[i];
            break;
          default:
            //接下来就处理一些特殊情况了  比如%02d之类的
            if(*fmt<='9'&&*fmt>='0'  &&  *(fmt+1)<='9'&&*(fmt+1)>'0'   &&  *(fmt+2)=='d' )
            {int val = va_arg(ap, int);     int ne_flag=0;
            if(val<0){ne_flag=1;val=0-val;}   for(buf_len=0;val; val /= 10, buf_len++) buf[buf_len] = NUM_CHAR[val % 10];
            while(buf_len<(int)(*(fmt+1)-'0')){buf[buf_len]='0';buf_len++;}    if(ne_flag==1){buf[buf_len]='-'; buf_len++;}
            for(int i = buf_len - 1; i >= 0; i--)  out[len++] = buf[i];
            fmt+=2;
            }
            
            break;               
        }
      break; // case % 的break.
      case '\n':
        out[len++] = '\n';break;
      case '\t':
        out[len++] = '\t';break;
      case '\b':
        out[len++] = '\b';break;
      case '\0':
        out[len++] = '\0';break;
      default:
        out[len++] = *fmt;break;
    }
    fmt++;
  }
  out[len] = '\0';    // 这句千万不能漏.
  return len;
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
  //panic("Not implemented");
  va_list args;
  va_start(args,fmt);
  return vsnprintf(out,n,fmt,args);
}


#endif
