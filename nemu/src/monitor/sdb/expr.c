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
  TK_PLUS,  // 258
  TK_SUB,   // 259
  TK_MULTI, // 260
  TK_DIV,   // 261
  TK_HEX,
  TK_INT,
  TK_NEGA_INT,
  TK_REG,
  TK_BRACKET_LEFT,
  TK_BRACKET_RIGHT,
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" -[0-9]+", TK_NEGA_INT}, // nega int
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
// token len
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
        printf("%d\n", rules[i].token_type);
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        /*Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,*/
        /*    rules[i].regex, position, substr_len, substr_len, substr_start);*/

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
          copystr(&tokens[nr_token], substr_start + 2, substr_len - 2);
          break;
        case TK_INT:
          tokens[nr_token].type = TK_INT;
          copystr(&tokens[nr_token], substr_start, substr_len);
          break;
        case TK_NEGA_INT:
          tokens[nr_token].type = TK_NEGA_INT;
          copystr(&tokens[nr_token], substr_start, substr_len);
          break;
        case TK_EQ:
          tokens[nr_token].type = TK_EQ;
          break;
          // this 后面都会++，但是
        case TK_NOTYPE:
          nr_token--;
          break;
        case TK_REG:
          tokens[nr_token].type = TK_REG;
          copystr(&tokens[nr_token], substr_start + 1, substr_len - 1);
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
  for (int j = 0; j < nr_token; j++) {
    printf("%d---str:%s\n", tokens[j].type, tokens[j].str);
  }
  printf("tokens len:%d\n", nr_token);
  return true;
}

static void copystr(Token *token, char *substr_start, int substr_len) {
  char match[substr_len + 1];
  strncpy(match, substr_start, substr_len);
  match[substr_len] = '\0';
  strcpy(token->str, match);
}

static word_t str2num(char *text, int carry, bool *success) {
  word_t res = 0;
  int base = 1;
  int len = strlen(text);
  for (int i = len - 1; i >= 0; i--) {
    if ((text[i] & 0xf0) == 0x30) {
      res += (text[i] & 0x0f) * base;
      base = base * carry;
    } else {
      res += ((text[i] & 0x0f) + 9) * base;
      base = base * carry;
    }
  }
  return res;
}

static word_t getTokenValue(Token *token, bool *success) {
  switch (token->type) {
  case TK_INT:
    return str2num(token->str, 10, success);
  case TK_NEGA_INT: {
    word_t post_res = str2num(token->str + 2, 10, success);
    return (~post_res) + 1;
  }
  case TK_HEX:
    return str2num(token->str, 16, success);
  case TK_REG:
    return isa_reg_str2val(token->str, success);
  }
  return 0;
}

bool check_parentheses(int p, int q) {
  int flag_bracket = 0;
  if (tokens[p].type != TK_BRACKET_LEFT) {
    return false;
  }
  if (tokens[q].type != TK_BRACKET_RIGHT) {
    return false;
  }
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_BRACKET_LEFT)
      flag_bracket++;
    if (tokens[i].type == TK_BRACKET_RIGHT)
      flag_bracket--;
  }
  if (flag_bracket == 0) {
    return true;
  } else {
    return false;
  }
}

int find_main_position(int p, int q, bool *success) {
  // 如果是非括号的+ - 就可以直接认定为主运算符(buxing 最后的才行)
  // 如果遇到左括号就一直接加知道和右括号匹配
  // mark left bracket
  int op = p;
  int flag_bracket = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type >= 262 && tokens[i].type <= 264)
      continue;
    if (tokens[i].type == TK_BRACKET_LEFT) {
      flag_bracket++;
      continue;
    }
    if (tokens[i].type == TK_BRACKET_RIGHT) {
      flag_bracket--;
      if (flag_bracket < 0) {
        printf("bad exper!!\n");
        *success = false;
        return 0;
      }
      continue;
    }
    if (tokens[i].type == TK_PLUS || tokens[i].type == TK_SUB) {
      if (flag_bracket == 0) {
        /*printf("into tk plus:%d\n", i);*/
        op = i;
      }
    }
    if (tokens[i].type == TK_MULTI || tokens[i].type == TK_DIV) {
      if (flag_bracket == 0) {
        if (!(tokens[op].type == TK_PLUS || tokens[op].type == TK_SUB)) {
          /*printf("into tk mulit:%d\n", i);*/
          op = i;
        }
      }
    }
  }
  return op;
}

// 在递归是的异常处理
word_t eval(int p, int q, bool *success) {
  if (p > q) {
    printf("bad expression!!\n");
    *success = false;
    return 0;
  } else if (p == q) {
    /*return tokens[q];*/
    return getTokenValue(&tokens[p], success);
  } else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1, success);
  } else {
    // 找主运算符的位置
    // 5*(123+123)*4
    int op = find_main_position(p, q, success);
    /*printf("main_postion:%d\n", op);*/
    int val1 = eval(p, op - 1, success);
    int val2 = eval(op + 1, q, success);
    switch (tokens[op].type) {
    case TK_PLUS:
      return val1 + val2;
    case TK_SUB:
      return val1 - val2;
    case TK_MULTI:
      return val1 * val2;
    case TK_DIV:
      return val1 - val2;
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  printf("make tokens:%s\n", e);
  if (!make_token(e)) {
    printf("make invaild tokens!!");
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  /*return 0;*/
  return eval(0, nr_token - 1, success);
}
