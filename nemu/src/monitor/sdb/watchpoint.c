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
static WP *head = NULL, *free_ = NULL;

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

void deletewp(uint32_t N) {}
