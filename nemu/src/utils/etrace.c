#include <common.h>
#include <isa.h>
#include <elf.h>
#include <etrace.h>
#include <stdlib.h>
#include <memory/paddr.h>



void etrace_errors(word_t NO, vaddr_t epc)
{
    printf("触发异常的原因NO为:%d\n",NO);
    switch (NO)
    {case -1: printf("触发异常的具体原因为:执行yeild自陷指令\n"); break;
    default: break;}
    printf("触发异常的pc为:%#x\n",epc);
    printf("异常入口地址为:%#x\n",cpu.CSRs.mtvec);
}





