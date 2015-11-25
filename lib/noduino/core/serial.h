/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 *  This file is ported from the esp8266 core for Arduino environment
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "noduino.h"

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x10
#define SERIAL_6N1 0x14
#define SERIAL_7N1 0x18
#define SERIAL_8N1 0x1c
#define SERIAL_5N2 0x30
#define SERIAL_6N2 0x34
#define SERIAL_7N2 0x38
#define SERIAL_8N2 0x3c
#define SERIAL_5E1 0x12
#define SERIAL_6E1 0x16
#define SERIAL_7E1 0x1a
#define SERIAL_8E1 0x1e
#define SERIAL_5E2 0x32
#define SERIAL_6E2 0x36
#define SERIAL_7E2 0x3a
#define SERIAL_8E2 0x3e
#define SERIAL_5O1 0x13
#define SERIAL_6O1 0x17
#define SERIAL_7O1 0x1b
#define SERIAL_8O1 0x1f
#define SERIAL_5O2 0x33
#define SERIAL_6O2 0x37
#define SERIAL_7O2 0x3b
#define SERIAL_8O2 0x3f

#define SERIAL_FULL     0
#define SERIAL_RX_ONLY  1
#define SERIAL_TX_ONLY  2

#define	UART0		0
#define	UART1		1
#define UART_NO		-1

#define UART_TX_FIFO_SIZE	0x80
#define RING_BUFFER_SIZE	256

/* FIFO Ring Buffer */
typedef struct ring_buffer {
	unsigned char buffer[RING_BUFFER_SIZE];
	volatile unsigned int head;
	volatile unsigned int tail;
} ring_buffer_t;

typedef struct uart_ {
	int uart_nr;
	int baud_rate;
	bool rxEnabled;
	bool txEnabled;
	uint8_t rxPin;
	uint8_t txPin;

	ring_buffer_t *_tx_buffer;
	ring_buffer_t *_rx_buffer;
	bool _written;
} uart_t;

void serial_begin(unsigned long baud);
void serial_end();
void serial_flush();
uint8_t serial_availableForWrite(void);
size_t serial_write(char c);
void serial_print(const char *buf);
#define	serial_printf(x, ...)		\
	do{char b[128]={0};sprintf(b,x,##__VA_ARGS__);serial_print(b);}while(0)

int serial_available(void);
uint8_t serial_read(void);

void serial1_begin(unsigned long baud);
void serial1_end();
void serial1_flush();
uint8_t serial1_availableForWrite();
size_t serial1_write(char c);
void serial1_print(const char *buf);
#define	serial1_printf(x, ...)		\
	do{char b[128]={0};sprintf(b,x,##__VA_ARGS__);serial1_print(b);}while(0)

void uart_set_debug(int uart_nr);
int uart_get_debug();

void uart_interrupt_handler(uart_t * uart);
void serial_rx_complete_irq(uart_t * uart, char c);
void serial_tx_empty_irq(uart_t * uart);
void uart_wait_for_tx_fifo(uart_t * uart, size_t size_needed);
size_t uart_get_tx_fifo_room(uart_t * uart);
void uart_wait_for_transmit(uart_t * uart);
void uart_transmit_char(uart_t * uart, char c);
void uart_transmit(uart_t * uart, const char *buf, size_t size);
void uart_flush(uart_t * uart);
void uart_interrupt_enable(uart_t * uart);
void uart_interrupt_disable(uart_t * uart);
void uart_arm_tx_interrupt(uart_t * uart);
void uart_disarm_tx_interrupt(uart_t * uart);
void uart_set_baudrate(uart_t * uart, int baud_rate);
int uart_get_baudrate(uart_t * uart);

uart_t *uart_init(int uart_nr, int baudrate, int config, int mode);
void uart_uninit(uart_t * uart);
void uart_swap(uart_t * uart);

void uart_ignore_char(char c);
void uart0_write_char(char c);
void uart1_write_char(char c);

#endif
