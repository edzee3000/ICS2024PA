#include <stdio.h>
#include "assert.h"
#include <sys/time.h>
#include <time.h>
// 以毫秒为单位返回系统时间
#include "../../libs/libndl/include/NDL.h"

int main() {
  // struct timeval tv;
  // struct timezone tz;
  uint32_t ms = NDL_GetTicks()+500;
  while (1) {
    uint32_t now_ms = NDL_GetTicks();
    do{
      now_ms = NDL_GetTicks();
    }while(now_ms < ms); 
    printf("ms = %u\n", ms);
    ms += 500;
  }
  printf("执行完timer-test\n");
  return 0;
}

// int main() {
//   struct timeval tv;
//   // struct timezone tz;
//   gettimeofday(&tv, NULL);
//   int ms = 500;
//   while (1) {
//     while ((tv.tv_sec * 1000 + tv.tv_usec / 1000) < ms) {
//       gettimeofday(&tv, NULL);
//     }
//     printf("ms = %d\n", ms);
//     ms += 500;
//   }
//   printf("执行完timer-test\n");
//   return 0;
// }