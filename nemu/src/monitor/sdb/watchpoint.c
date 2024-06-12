/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* TODO: Add more members if necessary */
  word_t value;
  char *str;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL, *end = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].str = NULL;
    wp_pool[i].value = 0;
  }

  head = wp_pool;
  free_ = wp_pool;
  end = &wp_pool[NR_WP - 1];
}

/* TODO: Implement the functionality of watchpoint */
// add wp
void addWP(char *exper, bool *success) {
  free_->str = exper;
  free_->value = expr(exper, success);
  if (!(*success)) {
    printf("add wp invaild!!\n");
    return;
  }
  printf("exper:%s,value:%x\n", free_->str, free_->value);
  free_ = free_->next;
}

void infoWP() {
  WP *n_head = head;
  while (n_head != free_) {
    printf("num %d:exper:%s,value:%x\n", n_head->NO, n_head->str,
           n_head->value);
    n_head++;
  }
}
// execute wp every time when starting
void execute_wp() {}

// static link
void deleteWP(uint32_t N, bool *success) {
  // find the spec nodes by n
  // mid preivous nodes node->next = node->next->next
  //  将节点挂在队尾才行
  printf("%d\n", N);
  if (N == 0) {
    end->next = head;
    head = head->next;
    end = end->next;
    end->next = NULL;
    return;
  }
  WP *n_head = head;
  uint32_t i = 0;
  while (i < N && n_head != free_) {
    printf("into loop!!\n");
    if (N == i + 1) {
      WP *temp = n_head->next;
      // nullpointer exception
      printf("num %d:exper:%s,value:%x\n", n_head->NO, n_head->str,
             n_head->value);
      printf("num %d:exper:%s,value:%x\n", temp->NO, temp->str, temp->value);
      n_head->next = n_head->next->next;
      end->next = temp;
      temp->next = NULL;
      end = end->next;
      return;
    }
    n_head = n_head->next;
    i++;
  }
  *success = false;
}
