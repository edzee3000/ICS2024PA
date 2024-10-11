#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdlib.h>
// #include <regex.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
// #if !defined(__ISA_NATIVE__) || !defined(__NATIVE_USE_KLIB__)
#define MAX_LEN_INPUT (uint32_t)(65536)
char* itoa(int num,char* str,int radix);
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



char* itoa(int num,char* str,int radix)
{
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
	unsigned unum;//存放要转换的整数的绝对值,转换的整数可能是负数
	int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
 
	//获取要转换的整数的绝对值
	if(radix==10&&num<0)//要转换成十进制数并且是负数
	{
		unum=(unsigned)-num;//将num的绝对值赋给unum
		str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
	}
	else unum=(unsigned)num;//若是num为正，直接赋值给unum
 
	//转换部分，注意转换后是逆序的
	do
	{
		str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
		unum/=radix;//unum去掉最后一位
 
	}while(unum);//直至unum为0退出循环
 
	str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
 
	//将顺序调整过来
	if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
	else k=0;//不是负数，全部都要调整
 
	char temp;//临时变量，交换两个值时用到
	for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
	{
		temp=str[j];//头部赋值给临时变量
		str[j]=str[i-1+k-j];//尾部赋值给头部
		str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
	}
 
	return str;//返回转换后的字符串
}

#endif
