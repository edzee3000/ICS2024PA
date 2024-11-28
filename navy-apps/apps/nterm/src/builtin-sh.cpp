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
  if (strtok(str, " ") == NULL) return i;
  else i++;
  while (strtok(NULL, " ") != NULL) i++;
  return i;
}

static void get_argv(char *cmd, char **argv)
{ int argc = 0;
  argv[argc++] = strtok(cmd, " ");
  while (true)
  {argv[argc++] = strtok(NULL, " ");
  if (argv[argc - 1] == NULL) break;
  }
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

  char cmd_cpy[strlen(cmd) + 1];
  char *extract = strtok(strcpy(cmd_cpy, cmd), "\n");
  char *name = strtok(extract, " ");
  char *args = strtok(NULL, "");
  for (size_t i = 1; i < NR_CMD; i++)//按理来说应该从0开始，但是因为自己有一个my_echo函数因此不太想重构代码了那就从1开始吧
  {
    // printf("compare %s with %s\n", name, cmd_table[i].cmd_name);
    if (strcmp(name, cmd_table[i].cmd_name) == 0)
    {
      if (cmd_table[i].handler(args) < 0) sh_printf("sh: invalid command\n"); 
      return;
    }
  }
  printf("cmd_name:%s\n",name);
  
  extract = strtok(strcpy(cmd_cpy, cmd), "\n");
  int argc = get_argc(extract);
  char *(argv[argc + 1]) = {NULL};
  extract = strtok(strcpy(cmd_cpy, cmd), "\n");
  get_argv(extract, argv);
  if (execvp(argv[0], argv) < 0)//这里开始运行程序   调用到了相关库中的execvp函数   从而就可以触发navy当中的_syscall函数  然后nanoslite里面就可以触发SYS_execve
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
  printf("执行到my_exit\n");
  exit(0);
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