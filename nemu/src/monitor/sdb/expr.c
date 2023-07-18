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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,NUM=0,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"-", '-'},         // sub
  {"\\*", '*'},         // mul
  {"/", '/'},         // div
  {"[0-9]+", NUM},         // num
  {"==", TK_EQ},        // equal
  {"!=", TK_EQ},        // equal
  {"&&", TK_EQ},        // equal
  {"\\(", '('},         // (
  {"\\)", ')'},         // )
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

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
         case NUM :
	          Token t;	 
            t.type = rules[i].token_type;
            if (substr_len >25)
            {
              assert(0);
            }
            
            strncpy(t.str, substr_start,substr_len);
	          tokens[nr_token++] = t;
            break;
          default:
	          Token t1; 
            t1.type = rules[i].token_type;
            tokens[nr_token++] = t1;
            break;
	}

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


static bool check_parentheses(int p, int q){
  if (tokens[p].type != '(' || tokens[q].type != ')')
  {
    return false;
  }

  int top = -1;
  int leftIndex[65536];
  int i = 0;
  for (i = p; i <= q; i++)
  {
    if(tokens[i].type == '(')
    {
      leftIndex[++top] = i;
    }

    if(tokens[i].type == ')')
    {
      if (top == -1)
      {
        return false;
      }
      int index = leftIndex[top--];
      if (i == q)
      {
        if (index != p)
        {
          return false;
        }
        
      }
      
    }

  }
  if (top != -1)
  {
    return false;
  }
  return true;
}
static int getMainExprPosition(int p, int q){
  int i;
  int flag = 0;
  int left = -1;
  for(i=q;i>=p;i--){
    if(tokens[i].type == TK_NOTYPE || tokens[i].type == NUM){
        continue;;
    }
    if(tokens[i].type == ')'){
      flag++;
      continue;
    }
    if(tokens[i].type == '('){
      flag--;
      continue;
    }
    if(flag > 0){
      continue;
    }
    if(left==-1 &&( tokens[i].type == '*' || tokens[i].type == '/' )){
      left = i;
    }

    if(tokens[i].type == '+' || tokens[i].type == '-' ){
     left = i;
     return left;
    }
  }
  if (left == -1)
  {
    assert(0);
  }
  
  return left;

}


static int eval(int p, int q) {
  if (p > q) {
    assert(0);
  }
  else if (p == q) {
    return atoi(tokens[p].str);
  }

  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  else {
    int op = getMainExprPosition(p,q);
    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(0,nr_token-1);
}
