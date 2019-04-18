
#include "uart.h"
#include <stdio.h>
//#include <stdlib.h>
#include <avr/io.h>
#include <util/setbaud.h>

/*
typedef struct _char_queue {
    char c;
    struct _char_queue* next;
} char_queue_t;

static volatile char_queue_t* queue = NULL;
static volatile int queue_empty = 1;

int push_queue(char c) {
    char_queue_t* new_item;
    char_queue_t** tracer;
    int ret = 1;

    tracer = &queue;
    while(tracer) {
        tracer = &(*tracer)->next;
        ret++;
    }
    new_item = malloc(sizeof(char_queue_t));
    new_item->c = c;
    new_item->next = NULL;
    *tracer = new_item;
    return ret;
}

int pop_queue(char* c) {
    char_queue_t** tracer;
    char_queue_t* old;

    tracer = &queue;
    if(*tracer) {
        old = *tracer;
        *tracer = (*tracer)->next;
        c = old->c;
        free(old);
        return (*tracer)->next;
    }
    return 0;
}
*/

void uart_init(void) {
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;

    #if USE_2X
    UCSR1A |= _BV(U2X1);
    #else
    UCSR1A &= ~(_BV(U2X1));
    #endif

    //UCSR1C = _BV(URSEL) |  _BV(UCSZ11) | _BV(UCSZ10); // 8 bit, interrupt
    UCSR1C = _BV(UCSZ11) | _BV(UCSZ10); /* 8-bit data */
    UCSR1B = _BV(RXEN1) | _BV(TXEN1);   /* Enable RX and TX */
    

    //UCSR1B |= _BV(UDRIE1);
    //UCSR1A |= _BV(UDRE1);
}

void uart_putchar(char c) {
    loop_until_bit_is_set(UCSR1A, UDRE1);
    UDR1 = c;
}

char uart_getchar(void) {
    loop_until_bit_is_set(UCSR1A, RXC1);
    return UDR1;
}

int uart_available(void) {
    return (UCSR1A & (1<<RXC1));
}


// these all have to do with attaching the uart to stdin/stdout
static void _putchar(char c, FILE* stream) {
    if(c == '\n') {
        uart_putchar('\r');
    }
    uart_putchar(c);
}
static char _getchar(FILE* stream) {
    return uart_getchar();
}

FILE uart_output = FDEV_SETUP_STREAM(_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input  = FDEV_SETUP_STREAM(NULL, _getchar, _FDEV_SETUP_READ);
FILE uart_io     = FDEV_SETUP_STREAM(_putchar, _getchar, _FDEV_SETUP_RW);

void uart_attach_std(void) {
    stdout = &uart_output;
    stdin  = &uart_input;
}
