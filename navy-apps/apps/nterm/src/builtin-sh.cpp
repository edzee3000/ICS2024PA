#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

#include <stdio.h>
#include <string.h>

char handle_key(SDL_Event *ev);
#define NR_CMD sizeof(cmd_table) / sizeof(cmd_table[0])
typedef int (*handler_t)(const char *);
static int my_echo(const char *cmd);
static int my_exit(const char *args);

static size_t get_argc(char *str);
static void get_argv(char *cmd, char **argv);


#define STRCPY strcpy(cmd_cpy, cmd)




//###################################################################################################
static struct
{
  const char *cmd_name;
  handler_t handler;
} cmd_table[] = {
    {"echo", my_echo},
    {"exit", my_exit},
    
};

static size_t get_argc(char *str)
{ size_t i = 0;
  if (strtok(str, " ") == NULL) return i;//特别要注意分割处理后原字符串 str 会变，变成第一个子字符串
  else i++;
  while (strtok(NULL, " ") != NULL) i++;
  printf("在builtin-sh当中argc参数数量为:%d\n",i);
  return i;
}

static void get_argv(char *cmd, char **argv)
{ int arg_num = 0;
  argv[arg_num++] = strtok(cmd, " ");//argv[arg_num++]中的arg_num是先用后自增，但是最好别这样写
  while (true)
  {argv[arg_num++] = strtok(NULL, " ");
  if (argv[arg_num - 1] == NULL) break;//因为到了最后没有办法分割的时候会返回NULL，因此可以用argnum-1的索引==NULL去判断是否已经分割完毕
  }
  for(int i=0;i<arg_num-1;i++){printf("argv[%d]的内容为:%s\n",i,argv[i]);}
  return;
}


static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  
  if(!cmd) return;
  if(my_echo(cmd)==0) return ;

  char* cmd_cpy=(char *)malloc((strlen(cmd) + 1)*sizeof(char));
  // char cmd_cpy[strlen(cmd) + 1];
  char *extract = strtok(STRCPY, "\n");//特别要注意分割处理后原字符串 str 会变，变成第一个子字符串
  char *name = strtok(extract, " ");
  char *args = strtok(NULL, "");
  for (size_t i = 1; i < NR_CMD; i++)//按理来说应该从0开始，但是因为自己有一个my_echo函数因此不太想重构代码了那就从1开始吧
  {
    // printf("compare %s with %s\n", name, cmd_table[i].cmd_name);
    if (strcmp(name, cmd_table[i].cmd_name) == 0)
    {
      if (cmd_table[i].handler(args) < 0) sh_printf("sh: invalid args\n"); 
      return;
    }
  }
  printf("cmd_name:%s\n",name);
  
  //特别要注意分割处理后原字符串 str 会变，变成第一个子字符串  因此需要多次strcpy
  extract = strtok(STRCPY, "\n");//使用strtok函数进行提取字符串
  int argc = get_argc(extract);
  char *(argv[argc + 1]) = {NULL};
  extract = strtok(STRCPY, "\n");
  get_argv(extract, argv);
  
  // 你只需要通过setenv()函数来设置PATH=/bin, 然后调用execvp()来执行新程序即可. 调用setenv()时需要将overwrite参数设置为0, 这是为了可以在Navy native上实现同样的效果.
  int execve_status=execvp(argv[0], argv);//这里开始运行程序   调用到了相关库中的execvp函数   从而就可以触发navy当中的_syscall函数  然后nanoslite里面就可以触发SYS_execve
  free(cmd_cpy);//注意这里的free(cmd_cpy);要放在execvp(argv[0], argv);之后，否则会产生错误指针
  if (execve_status < 0)
    sh_printf("sh: command not found: %s\n", argv[0]);
  return;
}
//###################################################################################################





void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  //##########这里会不会有问题？？###########
  setenv("PATH", "/bin", 0);//键入命令的完整路径是一件相对繁琐的事情. 回想我们使用ls的时候, 并不需要键入/bin/ls. 这是因为系统中定义了PATH这个环境变量, 你可以通过man execvp来阅读相关的行为. 我们也可以让NTerm中的內建Shell支持这一功能, 你只需要通过setenv()函数来设置PATH=/bin, 然后调用execvp()来执行新程序即可. 调用setenv()时需要将overwrite参数设置为0, 这是为了可以在Navy native上实现同样的效果.
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}

 

static int my_exit(const char *args)
{
  int exit_status=0;
  if(args!=0)exit_status=atoi(args);
  printf("执行到my_exit\t退出状态为:%d\n",exit_status);
  exit(exit_status);
  // exit(0);
  return 0;
}

//实现一个內建的echo命令
static int my_echo(const char *cmd)
{
  if(strncmp(cmd,"echo ",5)!=0) return -1;//这里需要将空格也要考虑进来
  uint32_t cmd_len=strlen(cmd)-1;
  while(cmd[cmd_len-1]==' '){cmd_len--;}//除去末尾多余空格的情况
  uint32_t output_strlen=cmd_len-5;//注意cmd_len长度是不包含'\0'的
  // printf("cmd_len:%u\n",cmd_len);
  const char *output_str=&cmd[5];
  uint32_t index=5;while(*output_str==' ') {output_str++;output_strlen--;index++;}//除去多余空格的情况
  //这里出了问题是因为cmd[index] =='\"'&& cmd[cmd_len-1]=='\"'写成了cmd[index] ==  cmd[cmd_len-1]=='\"'  这个是有问题的！！！！！！！
  if(cmd[index] =='\"'&& cmd[cmd_len-1]=='\"' || cmd[index]=='\''&& cmd[cmd_len-1]=='\''){output_strlen-=2; output_str++;}//除去第一对引号
  char arr[256]={0};
  strncpy(arr,output_str,output_strlen);
  arr[output_strlen]='\0';
  // printf("output_strlen:%u\n",output_strlen);
  sh_printf("%s\n",arr);
  return 0;
}