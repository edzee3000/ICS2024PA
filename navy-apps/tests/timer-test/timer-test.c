#include <stdio.h>
#include "assert.h"
#include <sys/time.h>
#include <time.h>

int main() {
  struct timeval tv;
  // struct timezone tz;
  gettimeofday(&tv, NULL);
  int ms = 500;
  while (1) {
    while ((tv.tv_sec * 1000 + tv.tv_usec / 1000) < ms) {
      gettimeofday(&tv, NULL);
    }
    printf("time = %.1f\n", ((float)ms)/1000);
    ms += 500;
  }
  printf("执行完timer-test\n");
  return 0;
}