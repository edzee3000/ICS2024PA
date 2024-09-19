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

#include "sdb.h"

#define NR_WP 32

#include <string.h>
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char EXPR[128];
  uint32_t Res;
} WP;


static WP* new_wp();//其中new_wp()从free_链表中返回一个空闲的监视点结构, 
//需要注意的是, 调用new_wp()时可能会出现没有空闲监视点结构的情况, 为了简单起见, 此时可以通过assert(0)马上终止程序
static void free_wp(WP *wp);//free_wp()将wp归还到free_链表中
void print_wp();
void set_watch_pointer(char *args,uint32_t res);
void delete_N_wp(int N);
void wp_diff_test();




static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;//其中head用于组织使用中的监视点结构, free_用于组织空闲的监视点结构, init_wp_pool()函数会对两个链表进行初始化.

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {//（默认监视点序号都是从开始计算的）
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;//head用于组织使用中的监视点结构
  free_ = wp_pool;//free_用于组织空闲的监视点结构

  // WP* temp=new_wp();
  // free_wp(temp);
  // print_wp();
}

/* TODO: Implement the functionality of watchpoint */
//为了使用监视点池, 你需要编写以下两个函数(你可以根据你的需要修改函数的参数和返回值):
static WP* new_wp(){
  if(free_==NULL)assert(0);//表明调用new_wp()时可能会出现没有空闲监视点结构的情况
	WP *temp;
	temp = free_;//从空闲的监视点池当中返回头指针对应的节点给temp然后返回
	free_ = free_->next;
	temp->next = NULL;
	if (head == NULL){
		head = temp;
	} 
  else {
		WP* temp2;
		temp2 = head;//把temp指针传给wp_pool池当中，并且是链表的最末端
		while (temp2->next != NULL){
			temp2 = temp2->next;
		}
		temp2->next = temp;
	}
	return temp;//最终返回temp监视点指针
}

static void free_wp(WP *wp){
	if (wp == NULL){
		assert(0);//表明已经没有可以再free的了
	}
	if (wp == head){
		head = head->next;//即使free之后没有节点了head就为NULL了
	} else {
		WP* temp = head;
		while (temp != NULL && temp->next != wp){
			temp = temp->next;
		}//当temp->next是wp的时候将temp->next指向wp->next
		temp->next = wp->next;
	}
	wp->next =free_;//将空闲的监视点加入到空闲的池子当中去，本质上空闲池和监视点池其实都是一个栈数据结构，只不过一个是以头进出，一个是以尾进出
	free_ = wp;
	// wp->result = 0;
	// wp->expr[0] = '\0';
}

void set_watch_pointer(char *args,uint32_t res)
{//传入表达式以及最终结果
WP* Insert=new_wp();
strcpy(Insert->EXPR,args);//复制表达式内容
(Insert->EXPR)[128]='\0';
Insert->Res=res;
}

void print_wp()
{
  WP* temp=head;
  if(temp==NULL){printf("目前没有设置监视点\n");return ;}
  while(temp!=NULL)
  {
    printf("NO.%d监视点的值为：%#x\t表达式为：%s\n",temp->NO,temp->Res,temp->EXPR);
    temp=temp->next;
  }
}
//删除序号为N的监视点
void delete_N_wp(int N)
{
//删除序号为N的监视点（默认监视点序号都是从开始计算的）
WP* temp=head;
while (temp!=NULL && temp->NO!=N)
{
	temp=temp->next;
}
if (temp==NULL)
{printf("找不到序号为%d的监视点",N);return ; }
free_wp(temp);
return;
}





//检查监视点是否发生变化
void wp_diff_test() {
  WP* temp = head;
  bool flag=false;//flag用于记录是否有改变有改变则为true
  while (temp!=NULL) {
    bool success=true;
    word_t new_res = expr(temp->EXPR, &success);
	if(!success){printf("表达式有误");assert(0);}
    if (temp->Res!=new_res) {
		flag=true;
      printf("序号为%d的监视点发生改变\n"
	  	"表达式为：%s\n"
        "原先值为：%u\t%#x\n"
        "现在值为：%u\t%#x\n"
        , temp->NO,temp->EXPR ,temp->Res,temp->Res, new_res,new_res);
      temp->Res=new_res;
    }
    temp=temp->next;
  }
	//printf("在此处插入一个测试点\n");
  if(flag){nemu_state.state=NEMU_STOP;}//程序因为触发了监视点而暂停了下来
}