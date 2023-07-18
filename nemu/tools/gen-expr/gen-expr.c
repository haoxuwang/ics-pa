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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


// 生成随机数字
char* gen_num() {
    char* num = malloc(2 * sizeof(char));  // 分配内存用于存储数字和结束符'\0'
    num[0] = '0' + rand() % 10;  // 生成范围为0-9的随机数字
    num[1] = '\0';  // 字符串结束符
    return num;
}

// 生成随机操作符
char* gen_rand_op() {
    char* ops = "+-*/";
    char* op = malloc(2 * sizeof(char));  // 分配内存用于存储操作符和结束符'\0'
    op[0] = ops[rand() % 4];  // 随机选择一个操作符
    op[1] = '\0';  // 字符串结束符
    return op;
}

// 生成随机表达式
char* gen_rand_expr() {
    char* a;
    switch (rand() % 3) {
        case 0:
            a = gen_num();
            break;
        case 1: {
            char* expr = gen_rand_expr();
            a = malloc((strlen(expr) + 3) * sizeof(char));  // 分配足够的内存用于存储括号和结束符'\0'
            sprintf(a, "(%s)", expr);  // 将表达式括在括号中
            free(expr);  // 释放之前分配的内存
            break;
        }
        default: {
            char* left_expr = gen_rand_expr();
            char* right_expr = gen_rand_expr();
            char* op = gen_rand_op();
            a = malloc((strlen(left_expr) + strlen(right_expr) + strlen(op) + 1) * sizeof(char));  // 分配足够的内存用于存储两个表达式、操作符和结束符'\0'
            strcpy(a, left_expr);  // 复制左表达式
            strcat(a, op);  // 连接操作符
            strcat(a, right_expr);  // 连接右表达式
            free(left_expr);  // 释放之前分配的内存
            free(right_expr);  // 释放之前分配的内存
            free(op);  // 释放之前分配的内存
            break;
        }
    }
    return a;
}


int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    char* a = gen_rand_expr();
    while (strlen(a)>65536)
    {
        a = gen_rand_expr();
    }
    

    //printf("gen: %ld\n", strlen(a));
    strcpy(buf,a);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0)
    {
        i--;
        continue;
    }
    
	if (WIFEXITED(ret))
	{
		if (0 != WEXITSTATUS(ret))
		{
            i--;
			continue;
		}
	}
	else
	{
        i--;
		continue;
	}

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);
        
    if (ret != EOF) 
      printf("%u %s\n", result, buf);
    else
      i--;
  }
  return 0;
}
