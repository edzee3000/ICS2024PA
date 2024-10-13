#ifndef __IRINGBUF_H__
#define __IRINGBUF_H__


#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
#include <utils.h>

#define MAX_NUM_NODE 16
typedef struct IringNode{
  word_t pc;
  uint32_t instr;
  struct IringNode *next;
}IringNode;

void Update_Buf(word_t pc,uint32_t instr);
void display_iringbuf();


#endif