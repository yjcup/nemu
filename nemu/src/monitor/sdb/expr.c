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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_PLUS,
  TK_SUB,
  TK_MULTI,
  TK_DIV,
  TK_HEX,
  TK_INT,
  TK_BRACKET_LEFT,
  TK_BRACKET_RIGHT,
  TK_REG
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},         // spaces
    {"\\+", TK_PLUS},          // plus
    {"==", TK_EQ},             // equal
    {"-", TK_SUB},             // sub
    {"\\*", TK_MULTI},         // equal
    {"\\\\", TK_DIV},          // div
    {"\\(", TK_BRACKET_LEFT},  // bracket
    {"\\)", TK_BRACKET_RIGHT}, // bracket
    {"\\b[0-9]+\\b", TK_INT},  // int
    {"0x[0-9a-z]+", TK_HEX},   // hex
    {"\\$[0-9a-z]+", TK_REG},  // regs
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      printf("%d\n", i);
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;
static void copystr(Token *token, char *substr_start, int substr_len);

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_PLUS:
          tokens[nr_token].type = TK_PLUS;
          break;
        case TK_SUB:
          tokens[nr_token].type = TK_SUB;
          break;
        case TK_MULTI:
          tokens[nr_token].type = TK_MULTI;
          break;
        case TK_DIV:
          tokens[nr_token].type = TK_SUB;
          break;
        case TK_BRACKET_LEFT:
          tokens[nr_token].type = TK_BRACKET_LEFT;
          break;
        case TK_BRACKET_RIGHT:
          tokens[nr_token].type = TK_BRACKET_RIGHT;
          break;
        case TK_HEX:
          tokens[nr_token].type = TK_HEX;
          copystr(&tokens[nr_token], substr_start, substr_len);
          break;
        case TK_INT:
          tokens[nr_token].type = TK_INT;
          copystr(&tokens[nr_token], substr_start, substr_len);
          break;
        case TK_EQ:
          tokens[nr_token].type = TK_EQ;
          break;
          copystr(&tokens[nr_token], substr_start, substr_len);
          // this 后面都会++，但是
        case TK_NOTYPE:
          nr_token--;
          break;
        case TK_REG:
          tokens[nr_token].type = TK_REG;
          break;
        }
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  for (int i = 0; i < 32; i++) {
    printf("%d---str:%s\n", tokens[i].type, tokens[i].str);
  }

  return true;
}

static void copystr(Token *token, char *substr_start, int substr_len) {
  char match[substr_len + 1];
  strncpy(match, substr_start, substr_len);
  match[substr_len] = '\0';
  strcpy(token->str, match);
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  /*TODO();*/

  return 0;
}
