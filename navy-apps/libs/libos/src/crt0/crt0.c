#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

//注意 char *argv[]与char *envp[]是一个指针数组，本质上argv与envp都是一个二级指针，自增的时候加的是char*的大小
int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  // char *empty[] =  {NULL };
  // environ = empty;
  // exit(main(0, empty, empty));
  int argc=  *((int *)args);//将32位4字节的args指针的第一个进行解引用
  printf("args指针值为: %p\n", args);
  char **argv=(char **)(args+1);//注意这个时候我将args+1强制类型转换为了char**因为argv是一个指针数组
  printf("argv指针数组值为: %p\n",argv);
  //接下来循环就可以了根据下面那个示意图可以得出需要进行一个while循环知道遇到指针为char*是个NULL的时候
  char **temp=argv;//注意这里之所以是二级指针是因为++的时候需要加的是sizeof(char *)的大小而不是加sizeof(char)。在C语言中对一个指针进行自增操作时，结果是该指针向前移动了它所指向的数据类型的尺寸
  while(*temp!=NULL) temp++;
  char **envp=(++temp);printf("envp指针数组值为: %p\n", args);
  environ=envp;
  exit(main(argc, argv, envp));
  assert(0);//不应该到达这里
}
/*
          |               |
          +---------------+ <---- ustack.end
          |  Unspecified  |
          +---------------+
          |               | <----------+
          |    string     | <--------+ |
          |     area      | <------+ | |
          |               | <----+ | | |
          |               | <--+ | | | |
          +---------------+    | | | | |
          |  Unspecified  |    | | | | |
          +---------------+    | | | | |
          |     NULL      |    | | | | |
          +---------------+    | | | | |
          |    ......     |    | | | | |
          +---------------+    | | | | |
          |    envp[1]    | ---+ | | | |
          +---------------+      | | | |
          |    envp[0]    | -----+ | | |
          +---------------+        | | |
          |     NULL      |        | | |
          +---------------+        | | |
          | argv[argc-1]  | -------+ | |
          +---------------+          | |
          |    ......     |          | |
          +---------------+          | |
          |    argv[1]    | ---------+ |
          +---------------+            |
          |    argv[0]    | -----------+
          +---------------+
          |      argc     |
          +---------------+ <---- cp->GPRx
          |               |
*/


/*
然后修改call_main()的代码, 让它解析出真正的argc/argv/envp, 并调用main():
void call_main(uintptr_t *args) {
  argc = ???
  argv = ???
  envp = ???
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
*/
