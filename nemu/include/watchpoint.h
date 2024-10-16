#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>
#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
// #include <sdb.h>
#include <string.h>


#define NR_WP 32
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char EXPR[128];
  uint32_t Res;
} WP;


void print_wp();
void set_watch_pointer(char *args,uint32_t res);
void delete_N_wp(int N);
void wp_diff_test();




#endif