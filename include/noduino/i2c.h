/*
 * Copyright (c) 2015 - 2025 MaiKe Labs
 *
 * Software I2C library for esp8266
 * 
 * Porting from the project named esp8266 universal io bridge.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __I2C_H__
#define __I2C_H__

#include "noduino.h"

typedef enum
{
	i2c_state_invalid = 0,
	i2c_state_idle,
	i2c_state_header_send,
	i2c_state_start_send,
	i2c_state_bus_wait_1,
	i2c_state_bus_wait_2,
	i2c_state_address_send,
	i2c_state_address_ack_receive,
	i2c_state_address_ack_received,
	i2c_state_data_send_data,
	i2c_state_data_send_ack_receive,
	i2c_state_data_send_ack_received,
	i2c_state_data_receive_data,
	i2c_state_data_receive_ack_send,
	i2c_state_stop_send,
	i2c_state_error,
	i2c_state_size = i2c_state_error
} i2c_state_t;

_Static_assert(sizeof(i2c_state_t) == 4, "sizeof(i2c_state_t) != 4");

typedef enum
{
	i2c_error_ok = 0,
	i2c_error_no_init,
	i2c_error_invalid_state_not_idle,
	i2c_error_invalid_state_idle,
	i2c_error_invalid_state_not_send_header,
	i2c_error_invalid_state_not_send_start,
	i2c_error_invalid_state_not_send_address_or_data,
	i2c_error_invalid_state_not_receive_ack,
	i2c_error_invalid_state_not_send_ack,
	i2c_error_invalid_state_not_send_stop,
	i2c_error_bus_lock,
	i2c_error_sda_stuck,
	i2c_error_address_nak,
	i2c_error_data_nak,
	i2c_error_receive_error,
	i2c_error_device_error_1,
	i2c_error_device_error_2,
	i2c_error_device_error_3,
	i2c_error_device_error_4,
	i2c_error_device_error_5,
	i2c_error_error,
	i2c_error_size = i2c_error_error
} i2c_error_t;

_Static_assert(sizeof(i2c_error_t) == 4, "sizeof(i2c_error_t) != 4");

i2c_error_t i2c_init(uint32_t sda_index, uint32_t scl_index, uint32_t bit_delay);
i2c_error_t i2c_reset(void);
i2c_error_t i2c_send(uint32_t address, uint32_t length, const uint8_t *bytes);
i2c_error_t i2c_receive(uint32_t address, uint32_t length, uint8_t *bytes);
uint32_t i2c_error_format_string(const char *tag, i2c_error_t error, uint32_t size, char *dst);

i2c_error_t i2c_send_1(uint32_t address, uint32_t byte0);
i2c_error_t i2c_send_2(uint32_t address, uint32_t byte0, uint32_t byte1);
i2c_error_t i2c_send_3(uint32_t address, uint32_t byte0, uint32_t byte1, uint32_t byte2);
#endif
