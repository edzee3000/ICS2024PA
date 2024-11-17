#include <am.h>

#include <stdio.h>
#include <stdlib.h>

extern char _heap_start;
// int main(const char *args);

Area heap;

void putch(char ch) {
  // outb(SERIAL_PORT, ch);
  printf("%c",ch);
}

void halt(int code) {
 
  exit(code);

  // should not reach here
  while (1);
}
 