/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/alarm.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_HANDLER 8

//闹钟（定时器）设置和处理机制的实现

//这段代码的工作原理是：
//通过 add_alarm_handle 函数注册一个或多个处理函数到 handler 数组。
//init_alarm 函数初始化一个虚拟定时器，设置其周期性地发送 SIGVTALRM 信号。
//当定时器到期时，alarm_sig_handler 函数被调用，它执行所有注册的处理函数。


static alarm_handler_t handler[MAX_HANDLER] = {};//函数指针数组，用于存储多个闹钟处理函数
static int idx = 0;//用于跟踪 handler 数组中已存储的函数指针的数量

//添加闹钟处理函数， 这个函数接受一个 alarm_handler_t 类型的函数指针 h 作为参数，并将其添加到 handler 数组中。idx 用于记录当前已添加的处理函数的数量，并在添加新函数后自增
void add_alarm_handle(alarm_handler_t h) {
  assert(idx < MAX_HANDLER);
  handler[idx ++] = h;
}

//静态函数，用于处理闹钟信号 SIGVTALRM。当定时器到期时，系统会发送 SIGVTALRM 信号，该函数会被调用。
//它遍历 handler 数组，调用所有注册的处理函数。
static void alarm_sig_handler(int signum) {
  int i;
  for (i = 0; i < idx; i ++) {
    handler[i]();
  }
}

//这个函数用于初始化闹钟。它首先设置信号处理函数 alarm_sig_handler 来处理 SIGVTALRM 信号。
//然后，它配置了一个 itimerval 结构体，用于设置定时器的初始值和周期。
//it.it_value 设置了定时器的第一次触发时间，it.it_interval 设置了定时器的周期。
//这里，定时器的周期被设置为每秒钟触发 TIMER_HZ 次（TIMER_HZ 需要在其他地方定义，通常是一个大于0的整数）。
//最后，setitimer 函数用于启动定时器。
void init_alarm() {
  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_handler = alarm_sig_handler;
  int ret = sigaction(SIGVTALRM, &s, NULL);
  Assert(ret == 0, "Can not set signal handler");

  struct itimerval it = {};
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  it.it_interval = it.it_value;
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}
