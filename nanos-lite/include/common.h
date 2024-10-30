#ifndef __COMMON_H__
#define __COMMON_H__

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_CTE   //框架代码提供的操作系统nanos-lite还什么都没做  需要在nanos-lite/include/common.h中定义宏HAS_CTE
//#define HAS_VME
//#define MULTIPROGRAM
//#define TIME_SHARING

#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>


// 定义宏HAS_CTE, Nanos-lite会多进行以下操作:
// 初始化时调用init_irq()函数, 它将通过cte_init()函数初始化CTE
// 在panic()前调用yield()来触发自陷操作



#endif
