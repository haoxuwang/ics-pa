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

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  int preResult;
  char str[32];
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

int new_wp(char* exp, bool *success){
  if (free_ == NULL)
  {
    *success = false;
    return -1;
  }
  bool b = true;
  word_t a = expr(exp,&b);
  if (!b)
  {
    *success = false;
    return -1;
  }

  // head之前加了一个节点 为了统一操作
  WP new ;
  memset(&new, 0,sizeof(new)); 
  WP* last = &new;
  last->next = head;

  // last 指向到最后一个节点
  while (last->next !=NULL)
  {
    last = last->next;
  }

  // 加一个节点
  last->next = free_;
  if (head == NULL)
  {
    head = free_;
  }
  
  // 移动last指针
  last = last->next;
  // free后移一个节点
  free_ = free_->next;
  // 断开和free的链接
  last->next = NULL;
  
    
  strncpy(last->str,exp,strlen(exp));
  last->preResult = a;
  return last->NO;
  
}
void free_wp(int no, bool *success){
  // head之前加了一个节点 为了统一操作
  WP new ;
  memset(&new, 0,sizeof(new)); 
  WP* first = &new;
  first->next = head;

  WP* current = head;
  // 执行上一个节点
  WP* pre = first;
  while (current != NULL)
  {
    if (current->NO == no)
    {
      // 删除节点
      pre->next = current->next;

      // 把删除的节点加入到free中
      WP* f = free_;
      if (f->next != NULL)
      {
        f = f->next;
      }
      f->next = current;
      current->next = NULL;

      // 重置头结点
      head = first->next;
      *success = true;
      return;
    }

    pre = current;
    current = current->next;
  }


  *success = false;
  return;
}

bool execWatchPoint(){
  bool b = false;

  while (head != NULL)
  {
    bool c = true;
    word_t a = expr(head->str,&c);
    if(head->preResult != a){
      b = true;
      return b;
    }
    head = head->next;
  }
  

  return b;
}

void watch_display(){
  
}

/* TODO: Implement the functionality of watchpoint */

