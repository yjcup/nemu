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
#include "utils.h"
#include <cpu/cpu.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <regex.h>
#include <stdint.h>
#include <time.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
/*typedef struct token {*/
/*  int type;*/
/*  char str[32];*/
/*} Token;*/
/*static Token token[100];*/
/*static int flag = 0;*/

/*static uint32_t char2num(char *c) {*/
/*  uint32_t res = 0;*/
/*  int carry = 1;*/
/*  int len = strlen(c);*/
/*  for (int i = len - 1; i >= 0; i--) {*/
/*    res += (c[i] & 0x0f) * carry;*/
/*    carry *= 10;*/
/*  }*/
/*  return res;*/
/*}*/

/*static char check_parnetheses(int p, int q) { return 0; }*/
/**/
/*static int eval(int p, int q) {*/
/*  if (p > q) {*/
/*    printf("bad expression!!\n");*/
/*  } else if (p == q) {*/
/*    return token[q].type;*/
/*  } else if (check_parnetheses(p, q)) {*/
/*    return eval(p + 1, q - 1);*/
/*  } else {*/
/*  }*/
/*  return 0;*/
/*}*/

/*static char isNum(char c) {*/
/*  char c_f = c & 0x0f;*/
/*  char c_e = c & 0xf0;*/
/*  printf("isnum:%x\n", c_e);*/
/*  if (c_f <= 9 && c_f >= 0 && c_e == 0x30) {*/
/*    return 1;*/
/*  } else {*/
/*    return 0;*/
/*  }*/
/*}*/
// int hex reg constant  + - * - () space
/*static char signarr[] = {'+', '-', '*', '/', '(', ')'};*/
/**/
/*static char *pattern_arr[] = {"\\b[0-9]+\\b", "0x[0-9a-z]+",
 * "\\$[0-9a-z]+"};*/
/*static char *reg_text_p = NULL;*/
/*static int reg_match_len = 0;*/
/*static regmatch_t p_match[1];*/
/*static void update_regex(char *text);*/
/*static const int regex_len = sizeof(pattern_arr) / 8;*/
/*static char regex_error_message[100];*/
/*static regex_t regex_arr[sizeof(pattern_arr) / 8];*/

/*static void initreg(char *text) {*/
/*  reg_text_p = text;*/
/*  printf("%d\n", regex_len);*/
/*  for (int i = 0; i < regex_len; i++) {*/
/*    int regexinit = regcomp(&regex_arr[i], pattern_arr[i], REG_EXTENDED);*/
/*    if (regexinit) {*/
/*      printf("regex init failed\n");*/
/*      regerror(regexinit, &regex_arr[i], regex_error_message,*/
/*               sizeof(regex_error_message));*/
/*      printf("%s\n", regex_error_message);*/
/*      return;*/
/*    }*/
/*  }*/
/*  update_regex(text);*/
/*}*/
/*static void update_regex(char *text) {*/
/*  for (int i = 0; i < regex_len; i++) {*/
/*    // attention:p_match offset*/
/*    if (regexec(&regex_arr[i], text, 1, p_match, 0) == 0) {*/
/*      char *new_p = text + p_match[0].rm_so;*/
/*      int len = p_match[0].rm_eo - p_match[0].rm_so;*/
/*      if (i == 0) {*/
/*        reg_text_p = new_p;*/
/*        reg_match_len = len;*/
/*      }*/
/*      if (new_p <= reg_text_p) {*/
/*        reg_text_p = new_p;*/
/*        reg_match_len = len;*/
/*      }*/
/*    }*/
/*  }*/
/*}*/
/**/
// 1. regexec
/*static void genToken(char *text) {*/
/*  int len = strlen(text);*/
/*  char *p = text;*/
/*  char loop_mark = 0;*/
/*  for (int i = 0; i < len; i++) {*/
/*    loop_mark = 0;*/
/*    if (reg_text_p == (p + i)) {*/
//
/*      token[flag].type = 0;*/
/*      char match[reg_match_len + 1];*/
/*      strncpy(match, p + i, reg_match_len);*/
/*      match[reg_match_len] = '\0';*/
/*      strcpy(token[flag].str, match);*/
/**/
/*      flag++;*/
/*      p += reg_match_len;*/
/*      i = i + reg_match_len - 1;*/
/*      update_regex(reg_text_p + reg_match_len);*/
/*      // 这里的i也要边*/
/*      continue;*/
/*    }*/
/*    if (p[i] == ' ')*/
/*      continue;*/
/*    for (int j = 0; j < strlen(signarr); j++) {*/
/*      if (signarr[j] == p[i]) {*/
/*        // mark*/
/*        token[flag].type = j + 1;*/
/*        token[flag].str[0] = p[i];*/
/*        loop_mark = 1;*/
/*        flag++;*/
/*        break;*/
/*      }*/
/*    }*/
/*    if (!loop_mark) {*/
/*      printf("invaild expression!!!\n");*/
/*      return;*/
/*    }*/
/*  }*/
/*  return;*/
/*}*/

/*int init(char *text) { return 0; }*/
// \$[0-9a-z]+
// 0x[0-9a-z]+
// [0-9]+

/* We use the `readline' library to provide more flexibility to read from
 * stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  /*nemu_state.halt_ret = 0;*/
  nemu_state.state = NEMU_QUIT;
  return -1;
  /*return 0;*/
}

static int cmd_si(char *args) {
  // extract the first argument
  char *arg = strtok(NULL, " ");
  if (args == NULL) {
    cpu_exec(1);
  } else {
    /*printf("%s\n", arg);*/
    int len = strlen(arg);
    uint64_t n = 0;
    int carry = 1;
    for (int i = len - 1; i >= 0; i--) {
      // determine if char is legal
      uint8_t num_str = arg[i] & 0xf;
      uint8_t num_str_start = arg[i] & 0xf0;
      if (num_str_start == 0x30 && num_str >= 0 && num_str <= 9) {
        printf("%u\n", num_str);
        n += num_str * carry;
        carry *= 10;
      } else {
        printf("invaild input!!!\n");
        return 0;
      }
    }
    cpu_exec(n);
  }
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("please input subcmd\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");
  if (strlen(arg) != 1) {
    printf("length:invaild input!!!\n");
  } else {
    switch (arg[0]) {
    case 'r':
      isa_reg_display();
      break;
    case 'w':
      printf("todo watchpoint!!!\n");
      break;
    default:
      printf("unknown command:invaild input !!!\n");
    }
  }

  return 0;
}
static int cmd_x(char *args) {
  // parse args
  if (args == NULL) {
    printf("please input subcmd\n");
  }
  char *arg = strtok(NULL, " ");
  printf("%s\n", arg);
  /*initreg(args);*/
  /*genToken(args);*/

  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "step", cmd_si},
    {"info", "print regs or point info", cmd_info},
    {"x", "displya memary", cmd_x},

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
