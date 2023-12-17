#ifndef _FFTRACER_H_
#define _FFTRACER_H_
#include <common.h>

void stack_return(paddr_t cur, paddr_t des);

void stack_call(paddr_t cur, paddr_t des);

void print_stack_trace();

#endif