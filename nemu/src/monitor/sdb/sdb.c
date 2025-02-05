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
#include "common.h"
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
      infoWP();
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
    return 0;
  }
  char *arg = strtok(NULL, " ");
  char *exper = arg + strlen(arg) + 1;
  /*printf("arg exper:%s\n", exper);*/
  bool success = true;
  word_t res = expr(exper, &success);
  printf("exper res:%#x\n", res);
  if (success == false) {
    printf("invaild exper,please input again!!\n");
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("please input subcmd\n");
    return 0;
  }
  bool success = true;
  word_t res = expr(args, &success);
  if (!success) {
    printf("invalid exper cmd_p\n");
  }
  printf("exper res:%#x\n", res);
  return 0;
}

static int cmd_del_w(char *args) {

  if (args == NULL) {
    printf("please input subcmd\n");
    return 0;
  }
  bool success = true;
  char *num = strtok(NULL, " ");
  word_t res = expr(num, &success);
  if (!success) {
    printf("invalid expr!!\n");
  }
  deleteWP((uint32_t)res, &success);
  return 0;
}

static int cmd_add_w(char *args) {

  if (args == NULL) {
    printf("please input subcmd\n");
    return 0;
  }
  bool success = true;
  addWP(args, &success);

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
    {"info", "print regs or point info(r,w)", cmd_info},
    {"x", "displya memary x N expr", cmd_x},
    {"p", "p expr", cmd_p},
    {"w", "w expr", cmd_add_w},
    {"d", "w N", cmd_del_w},

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
