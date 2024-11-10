#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

#include <stdio.h>
#include <string.h>

char handle_key(SDL_Event *ev);

static void my_echo(const char *cmd);


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
  my_echo(cmd);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

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



static void my_echo(const char *cmd)
{
  if(strncmp(cmd,"echo",4)!=0) return;
  uint32_t cmd_len=strlen(cmd)-1;
  while(cmd[cmd_len-1]==' '){cmd_len--;}//除去末尾多余空格的情况
  uint32_t output_strlen=cmd_len-5;//注意cmd_len长度是不包含'\0'的
  printf("cmd_len:%u\n",cmd_len);
  const char *output_str=&cmd[5];
  uint32_t index=5;while(*output_str==' ') {output_str++;output_strlen--;index++;}//除去多余空格的情况
  if(cmd[index] == cmd[cmd_len-1]=='\"' || cmd[index]== cmd[cmd_len-1]=='\''){output_strlen-=2; output_str++;}//除去第一对引号
  char arr[256]={0};
  strncpy(arr,output_str,output_strlen);
  arr[output_strlen]='\0';
  printf("output_strlen:%u\n",output_strlen);
  sh_printf("%s\n",arr);
}