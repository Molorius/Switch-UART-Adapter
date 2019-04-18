
#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);

void uart_init9(void);

void uart_attach_std(void);

void uart_putchar(char c);

char uart_getchar(void);

uint16_t uart_getchar9(void);

int uart_available(void);

#endif