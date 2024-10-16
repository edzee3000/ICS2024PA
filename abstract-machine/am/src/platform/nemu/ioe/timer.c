#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}
//这里可能会有一点点问题  因为是在nemu当中（好像要在跑分那里修改一下？）
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // uint32_t low = inl(RTC_ADDR);
  // uint32_t high = inl(RTC_ADDR+4);
  // 这里可能会很奇怪inl()是个什么玩意？？它的函数定义在abstract-machine/am/src/riscv/riscv.h当中
  // static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }  用来访问设备过程中读取设备地址的
  uint32_t high = inl(RTC_ADDR+4);
  uint32_t low = inl(RTC_ADDR);
  uptime->us=(uint64_t)low | ((uint64_t)high)<<32;
}
 
void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
