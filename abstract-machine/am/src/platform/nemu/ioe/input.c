#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t k=inl(KBD_ADDR);//KBD_ADDR定义在abstract-machine/am/src/platform/nemu/include/nemu.h当中，表示键盘在nemu当中的地址
  //仿照native中的input.c可以写出
  kbd->keydown = (k & KEYDOWN_MASK ? true : false);   //keydown为true时表示按下按键,否则表示释放按键. 
  kbd->keycode =  k & ~KEYDOWN_MASK;  //keycode为按键的断码, 没有按键时, keycode为AM_KEY_NONE.
}
