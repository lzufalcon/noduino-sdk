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

#include "noduino.h"

uart_t *uart0 = NULL;
uart_t *uart1 = NULL;

size_t ICACHE_FLASH_ATTR rbuf_get_size(ring_buffer_t * b)
{
	if (b->tail >= b->head)
		return b->tail - b->head;

	return RING_BUFFER_SIZE - (b->head - b->tail);
}

size_t ICACHE_FLASH_ATTR rbuf_room(ring_buffer_t * b)
{
	if (b->tail >= b->head) {
		return RING_BUFFER_SIZE - (b->tail - b->head) - 1;
	}
	if (b->head == b->tail) {
		return RING_BUFFER_SIZE;
	}
	return b->head - b->tail - 1;
}

bool ICACHE_FLASH_ATTR rbuf_empty(ring_buffer_t * b)
{
	return b->head == b->tail;
}

/* get data from head */
char ICACHE_FLASH_ATTR rbuf_get_char(ring_buffer_t * b)
{
	if (rbuf_get_size(b) == 0)
		return 0;

	char result = b->buffer[b->head];
	if (++(b->head) == RING_BUFFER_SIZE)
		b->head = 0;
	return result;
}

/* push data to tail */
char ICACHE_FLASH_ATTR rbuf_put_char(ring_buffer_t * b, unsigned char c)
{
	if(rbuf_room(b) == 0) return 0;

	b->buffer[b->tail] = c;
	if (++(b->tail) == RING_BUFFER_SIZE) b->tail = 0;
	return 1;
}

/*
 *  UART GPIOs
 *
 * UART0 TX: 1 or 2
 * UART0 RX: 3
 *
 * UART0 SWAP TX: 15
 * UART0 SWAP RX: 13
 *
 *
 * UART1 TX: 7 (NC) or 2
 * UART1 RX: 8 (NC)
 *
 * UART1 SWAP TX: 11 (NC)
 * UART1 SWAP RX: 6 (NC)
 *
 * NC = Not Connected to Module Pads --> No Access
 *
*/
void ICACHE_FLASH_ATTR uart_interrupt_handler(uart_t * uart)
{
	if (uart == NULL)
		return;

	// -------------- UART 0 --------------
	if (uart->uart_nr == 0) {
		if (uart->rxEnabled) {
			while (U0IS & (1 << UIFF)) {
				serial_rx_complete_irq(uart, (char)(U0F & 0xff));
				U0IC = (1 << UIFF);
			}
		}
		if (uart->txEnabled) {
			if (U0IS & (1 << UIFE)) {
				U0IC = (1 << UIFE);
				serial_tx_empty_irq(uart);
			}
		}
	} else if (uart->uart_nr == 1) {

		// -------------- UART 1 --------------
		if (uart->rxEnabled) {
			while (U1IS & (1 << UIFF)) {
				serial_rx_complete_irq(uart, (char)(U1F & 0xff));
				U1IC = (1 << UIFF);
			}
		}
		if (uart->txEnabled) {
			if (U1IS & (1 << UIFE)) {
				U1IC = (1 << UIFE);
				serial_tx_empty_irq(uart);
			}
		}
	}
}

void ICACHE_FLASH_ATTR uart_wait_for_tx_fifo(uart_t * uart, size_t size_needed)
{
	if (uart == NULL)
		return;
	if (uart->txEnabled) {
		while (true) {
			size_t tx_count = (USS(uart->uart_nr) >> USTXC) & 0xff;
			if (tx_count <= (UART_TX_FIFO_SIZE - size_needed))
				break;
		}
	}
}

size_t ICACHE_FLASH_ATTR uart_get_tx_fifo_room(uart_t * uart)
{
	if (uart == NULL)
		return 0;
	if (uart->txEnabled) {
		return UART_TX_FIFO_SIZE -
		    ((USS(uart->uart_nr) >> USTXC) & 0xff);
	}
	return 0;
}

void ICACHE_FLASH_ATTR uart_wait_for_transmit(uart_t * uart)
{
	if (uart == NULL)
		return;
	if (uart->txEnabled) {
		uart_wait_for_tx_fifo(uart, UART_TX_FIFO_SIZE);
	}
}

void ICACHE_FLASH_ATTR uart_transmit_char(uart_t * uart, char c)
{
	if (uart == NULL)
	{
		return;
	}
	if (uart->txEnabled) {
		USF(uart->uart_nr) = c;
	}
}

void ICACHE_FLASH_ATTR uart_transmit(uart_t * uart, const char *buf, size_t size)
{
	if (uart == NULL)
		return;
	if (uart->txEnabled) {
		while (size) {
			size_t part_size = (size > UART_TX_FIFO_SIZE) ?
								UART_TX_FIFO_SIZE : size;
			size -= part_size;
			uart_wait_for_tx_fifo(uart, part_size);
			for (; part_size; --part_size, ++buf)
				USF(uart->uart_nr) = *buf;
		}
	}
}

void ICACHE_FLASH_ATTR uart_flush(uart_t * uart)
{
	uint32_t tmp = 0x00000000;
	if (uart == NULL)
		return;
	if (uart->rxEnabled) {
		tmp |= (1 << UCRXRST);
	}
	if (uart->txEnabled) {
		tmp |= (1 << UCTXRST);
	}
	USC0(uart->uart_nr) |= (tmp);
	USC0(uart->uart_nr) &= ~(tmp);
}

void ICACHE_FLASH_ATTR uart_interrupt_enable(uart_t * uart)
{
	if (uart == NULL)
		return;
	USIC(uart->uart_nr) = 0x1ff;
	ETS_UART_INTR_ATTACH(&uart_interrupt_handler, uart);	// uart parameter is not osed in irq function!
	if (uart->rxEnabled) {
		USIE(uart->uart_nr) |= (1 << UIFF);
	}
	ETS_UART_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR uart_interrupt_disable(uart_t * uart)
{
	if (uart == NULL)
		return;
	if (uart->rxEnabled) {
		USIE(uart->uart_nr) &= ~(1 << UIFF);
	}
	if (uart->txEnabled) {
		USIE(uart->uart_nr) &= ~(1 << UIFE);
	}
	//ETS_UART_INTR_DISABLE(); // never disable irq complete may its needed by the other Serial Interface!
}

void ICACHE_FLASH_ATTR uart_arm_tx_interrupt(uart_t * uart)
{
	if (uart == NULL)
		return;
	if (uart->txEnabled) {
		USIE(uart->uart_nr) |= (1 << UIFE);
	}
}

void ICACHE_FLASH_ATTR uart_disarm_tx_interrupt(uart_t * uart)
{
	if (uart == NULL)
		return;
	if (uart->txEnabled) {
		USIE(uart->uart_nr) &= ~(1 << UIFE);
	}
}

void ICACHE_FLASH_ATTR uart_set_baudrate(uart_t * uart, int baud_rate)
{
	if (uart == NULL)
		return;
	uart->baud_rate = baud_rate;
	USD(uart->uart_nr) = (ESP8266_CLOCK / uart->baud_rate);
}

int ICACHE_FLASH_ATTR uart_get_baudrate(uart_t * uart)
{
	if (uart == NULL)
		return 0;
	return uart->baud_rate;
}

uart_t* ICACHE_FLASH_ATTR uart_init(int uart_nr, int baudrate, int config, int mode)
{
	uint32_t conf1 = 0x00000000;
	uart_t *uart = (uart_t *) os_malloc(sizeof(uart_t));
	if (uart == NULL) {
		return 0;
	}
	uart->uart_nr = uart_nr;
	switch (uart->uart_nr) {
	case UART0:
		uart->rxEnabled = (mode != SERIAL_TX_ONLY);
		uart->txEnabled = (mode != SERIAL_RX_ONLY);
		uart->rxPin = (uart->rxEnabled) ? 3 : 255;
		uart->txPin = (uart->txEnabled) ? 1 : 255;
		if (uart->rxEnabled)
			pinMode(uart->rxPin, SPECIAL);
		if (uart->txEnabled)
			pinMode(uart->txPin, SPECIAL);
		IOSWAP &= ~(1 << IOSWAPU0);
		break;
	case UART1:
		uart->rxEnabled = false;
		uart->txEnabled = (mode != SERIAL_RX_ONLY);
		uart->rxPin = 255;
		uart->txPin = (uart->txEnabled) ? 2 : 255;
		if (uart->txEnabled)
			pinMode(uart->txPin, SPECIAL);
		break;
	case UART_NO:
	default:
		// big fail!
		os_free(uart);
		return 0;
	}
	uart_set_baudrate(uart, baudrate);
	USC0(uart->uart_nr) = config;
	uart_flush(uart);
	uart_interrupt_enable(uart);
	if (uart->rxEnabled) {
		conf1 |= (0x01 << UCFFT);
	}
	if (uart->txEnabled) {
		conf1 |= (0x20 << UCFET);
	}
	USC1(uart->uart_nr) = conf1;
	return uart;
}

void ICACHE_FLASH_ATTR uart_uninit(uart_t * uart)
{
	if (uart == NULL)
		return;
	uart_interrupt_disable(uart);
	switch (uart->rxPin) {
	case 3:
		pinMode(3, INPUT);
		break;
	case 13:
		pinMode(13, INPUT);
		break;
	}
	switch (uart->txPin) {
	case 1:
		pinMode(1, INPUT);
		break;
	case 2:
		pinMode(2, INPUT);
		break;
	case 15:
		pinMode(15, INPUT);
		break;
	}
	os_free(uart);
}

void ICACHE_FLASH_ATTR uart_swap(uart_t * uart)
{
	if (uart == NULL)
		return;
	switch (uart->uart_nr) {
	case UART0:
		if ((uart->txPin == 1 && uart->txEnabled)
		    || (uart->rxPin == 3 && uart->rxEnabled)) {
			if (uart->txEnabled)
				pinMode(15, FUNCTION_4);	//TX
			if (uart->rxEnabled)
				pinMode(13, FUNCTION_4);	//RX
			IOSWAP |= (1 << IOSWAPU0);
			if (uart->txEnabled) {	//TX
				pinMode(1, INPUT);
				uart->txPin = 15;
			}
			if (uart->rxEnabled) {	//RX
				pinMode(3, INPUT);
				uart->rxPin = 13;
			}
		} else {
			if (uart->txEnabled)
				pinMode(1, SPECIAL);	//TX
			if (uart->rxEnabled)
				pinMode(3, SPECIAL);	//RX
			IOSWAP &= ~(1 << IOSWAPU0);
			if (uart->txEnabled) {	//TX
				pinMode(15, INPUT);
				uart->txPin = 1;
			}
			if (uart->rxEnabled) {	//RX
				pinMode(13, INPUT);	//RX
				uart->rxPin = 3;
			}
		}
		break;
	case UART1:

		// current no swap possible! see GPIO pins used by UART
		break;
	default:
		break;
	}
}

void ICACHE_FLASH_ATTR uart_ignore_char(char c)
{

}

void ICACHE_FLASH_ATTR uart0_write_char(char c)
{
	if (uart0 != NULL && uart0->txEnabled) {
		if (serial_availableForWrite() > 0) {
			if (c == '\n') {
				serial_write('\r');
			}
			serial_write(c);
			return;
		}
	}
	// wait for the Hardware FIFO
	while (true) {
		if (((USS(0) >> USTXC) & 0xff) <= (UART_TX_FIFO_SIZE - 2)) {
			break;
		}
	}
	if (c == '\n') {
		USF(0) = '\r';
	}
	USF(0) = c;
}

void ICACHE_FLASH_ATTR uart1_write_char(char c)
{
	if (uart1 != NULL && uart1->txEnabled) {
		if (serial1_availableForWrite() > 0) {
			if (c == '\n') {
				serial1_write('\r');
			}
			serial1_write(c);
			return;
		}
	}
	// wait for the Hardware FIFO
	while (true) {
		if (((USS(1) >> USTXC) & 0xff) <= (UART_TX_FIFO_SIZE - 2)) {
			break;
		}
	}
	if (c == '\n') {
		USF(1) = '\r';
	}
	USF(1) = c;
}

void ICACHE_FLASH_ATTR serial_rx_complete_irq(uart_t * uart, char c)
{
	if (uart->_rx_buffer) {
		rbuf_put_char(uart->_rx_buffer, c);
	}
}

void ICACHE_FLASH_ATTR serial_tx_empty_irq(uart_t * uart)
{
	if (uart == 0)
		return;
	if (uart->_tx_buffer == 0)
		return;
	size_t queued = rbuf_get_size(uart->_tx_buffer);
	if (!queued) {
		uart_disarm_tx_interrupt(uart);
		return;
	}
	size_t room = uart_get_tx_fifo_room(uart);
	int n = (queued < room) ? queued : room;
	while (n--) {
		uart_transmit_char(uart, rbuf_get_char(uart->_tx_buffer));
	}
}

static int s_uart_debug_nr = UART0;

/////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR uart_set_debug(int uart_nr)
{
	s_uart_debug_nr = uart_nr;
	switch (s_uart_debug_nr) {
	case UART0:
		system_set_os_print(1);
		ets_install_putc1((void *)&uart0_write_char);
		break;
	case UART1:
		system_set_os_print(1);
		ets_install_putc1((void *)&uart1_write_char);
		break;
	case UART_NO:
	default:
		system_set_os_print(0);
		ets_install_putc1((void *)&uart_ignore_char);
		break;
	}
}

int ICACHE_FLASH_ATTR uart_get_debug()
{
	return s_uart_debug_nr;
}

uart_t * ICACHE_FLASH_ATTR __serial_begin(int uart_nr, unsigned long baud)
{
	uart_t *u = uart_init(uart_nr, baud, SERIAL_8N1, SERIAL_FULL);
	if (u == 0) {
		return 0;
	}
	// disable debug for this interface
	if (uart_get_debug() == u->uart_nr) {
		uart_set_debug(UART_NO);
	}
	if (u->rxEnabled) {
		if (u->_rx_buffer)
			u->_rx_buffer = malloc(sizeof(ring_buffer_t));
		if (u->_rx_buffer != NULL) {
			u->_rx_buffer->head = 0;
			u->_rx_buffer->tail = 0;
		}
	}
	if (u->txEnabled) {
		if (u->_tx_buffer)
			u->_tx_buffer = malloc(sizeof(ring_buffer_t));
		if (u->_tx_buffer != NULL) {
			u->_tx_buffer->head = 0;
			u->_tx_buffer->tail = 0;
		}
	}
	u->_written = false;
	delay(1);
	return u;
}

void ICACHE_FLASH_ATTR __serial_end(uart_t *uart)
{
	if (uart_get_debug() == uart->uart_nr) {
		uart_set_debug(UART_NO);
	}
	uart_uninit(uart);
	if (uart->_rx_buffer)
		free(uart->_rx_buffer);
	if (uart->_tx_buffer)
		free(uart->_tx_buffer);
	free(uart);
}

/* serial1 can not rx data, just tx for debug */
int ICACHE_FLASH_ATTR serial_available(void)
{
	int result = 0;
	if (uart0 != NULL && uart0->rxEnabled) {
		result = rbuf_get_size(uart0->_rx_buffer);
	}
	if (!result) {
		optimistic_yield(USD(uart0->uart_nr) / 128);
	}
	return result;
}

/* serial1 can not rx data, just tx for debug */
uint8_t ICACHE_FLASH_ATTR serial_read(void)
{
	uart_t *_uart = uart0;
	if (_uart == NULL)
		return -1;
	if (_uart->rxEnabled) {
		return rbuf_get_char(_uart->_rx_buffer);
	} else {
		return -1;
	}
}

uint8_t ICACHE_FLASH_ATTR __serial_availableForWrite(uart_t *uart)
{
	if (uart == NULL)
		return 0;
	if (uart->txEnabled) {
		return rbuf_room(uart->_tx_buffer);
	} else {
		return 0;
	}
}

void ICACHE_FLASH_ATTR __serial_flush(uart_t *uart)
{
	if (uart == NULL)
		return;
	if (!uart->txEnabled)
		return;
	if (!uart->_written)
		return;
	while (rbuf_get_size(uart->_tx_buffer)
	       || uart_get_tx_fifo_room(uart) < UART_TX_FIFO_SIZE)
		yield();
	uart->_written = false;
}

size_t ICACHE_FLASH_ATTR __serial_write(uart_t *uart, char c)
{
	if (uart == NULL || !uart->txEnabled)
		return 0;
	uart->_written = true;
	size_t room = uart_get_tx_fifo_room(uart);
	if (room > 0 && rbuf_empty(uart->_tx_buffer)) {
		uart_transmit_char(uart, c);
		if (room < 10) {
			uart_arm_tx_interrupt(uart);
		}
		return 1;
	}
	while (rbuf_room(uart->_tx_buffer) == 0) {
		yield();
		uart_arm_tx_interrupt(uart);
	}
	rbuf_put_char(uart->_tx_buffer, c);
	return 1;
}

void ICACHE_FLASH_ATTR serial_begin(unsigned long baud)
{
	uart0 = __serial_begin(UART0, baud);
}

void ICACHE_FLASH_ATTR serial1_begin(unsigned long baud)
{
	uart1 = __serial_begin(UART1, baud);
}

void ICACHE_FLASH_ATTR serial_end()
{
	__serial_end(uart0);
}

void ICACHE_FLASH_ATTR serial1_end()
{
	__serial_end(uart0);
}

uint8_t ICACHE_FLASH_ATTR serial_availableForWrite()
{
	return __serial_availableForWrite(uart0);
}

uint8_t ICACHE_FLASH_ATTR serial1_availableForWrite()
{
	return __serial_availableForWrite(uart1);
}

void ICACHE_FLASH_ATTR serial_flush()
{
	__serial_flush(uart0);
}

void ICACHE_FLASH_ATTR serial1_flush()
{
	__serial_flush(uart1);
}

size_t ICACHE_FLASH_ATTR serial_write(char c)
{
	return __serial_write(uart0, c);
}

size_t ICACHE_FLASH_ATTR serial1_write(char c)
{
	return __serial_write(uart0, c);
}

void ICACHE_FLASH_ATTR serial_print(const char *buf)
{
	while(*buf != '\0') {
		serial_write(*buf);
		buf++;
	}
}

void ICACHE_FLASH_ATTR serial1_print(const char *buf)
{
	while(*buf != '\0') {
		serial1_write(*buf);
		buf++;
	}
}
