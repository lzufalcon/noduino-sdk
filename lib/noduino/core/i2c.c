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

#include "i2c.h"

static uint32_t i2c_sda;
static uint32_t i2c_scl;
static uint32_t transaction_bit_delay;
static i2c_state_t state = i2c_state_invalid;

typedef enum
{
	i2c_direction_receive,
	i2c_direction_send,
} i2c_direction_t;

_Static_assert(sizeof(i2c_direction_t) == 4, "sizeof(i2c_direction_t) != 4");

typedef enum
{
	i2c_config_idle_timeout = 256,			// * long delay  5 us = ~1280 ms
	i2c_config_scl_sampling_window = 32,	// * short delay 1 us =   ~32 ms
	i2c_config_sda_sampling_window = 32,	// * short delay 1 us =  ~ 32 ms
} i2c_config_t;

struct
{
	uint32_t init_done:1;
} i2c_flags;

static i2c_error_t send_bit(bool bit);

static const char *state_strings[i2c_state_size] =
{
	"invalid",
	"idle",
	"send header",
	"send start",
	"wait for bus to quiesce",
	"wait for bus to keep quiet",
	"send address",
	"address receive ack",
	"address received ack",
	"send data",
	"send data receive ack",
	"send data ack received",
	"receive data",
	"receive data send ack",
	"send stop"
};

static const char *error_strings[i2c_error_size] =
{
	"ok",
	"uninitialised",
	"state not idle",
	"state idle",
	"state not send header",
	"state not send start",
	"state not send address or data",
	"state not receive ack",
	"state not send ack",
	"state not send stop",
	"bus locked",
	"sda stuck",
	"address NAK",
	"data NAK",
	"receive error",
	"device specific error 1",
	"device specific error 2",
	"device specific error 3",
	"device specific error 4",
	"device specific error 5",
};

irom static const char *i2c_state_string(void)
{
	if(state < i2c_state_size)
		return(state_strings[state]);
	else
		return("<unknown state>");
}

irom static const char *i2c_error_string(i2c_error_t error)
{
	if(error < i2c_error_size)
		return(error_strings[error]);
	else
		return("<unknown error>");
}

irom uint32_t i2c_error_format_string(const char *tag, i2c_error_t error,
		uint32_t size, char *dst)
{
	return(snprintf(dst, size, "%s: bus error: %s (in bus state: %s)",
				tag, i2c_error_string(error), i2c_state_string()));
}

irom static inline void short_delay(void)
{
	os_delay_us(1);
}

irom static inline void i2c_delay(void)
{
	os_delay_us(transaction_bit_delay);
}

#define clear_sda()		(GPES = (1 << i2c_sda))
#define set_sda()		(GPEC = (1 << i2c_sda))
#define clear_sck()		(GPES = (1 << i2c_scl))
#define set_scl()		(GPEC = (1 << i2c_scl))
#define sda_is_set()	((GPI & (1 << i2c_sda)) != 0)
#define scl_is_set()	((GPI & (1 << i2c_scl)) != 0)

irom static i2c_error_t wait_idle(void)
{
	uint32_t current;

	for(current = i2c_config_idle_timeout; current > 0; current--)
	{
		if(scl_is_set())
			break;

		i2c_delay();
	}

	if(current == 0)
		return(i2c_error_bus_lock);

	return(i2c_error_ok);
}

irom static i2c_error_t send_start(void)
{
	i2c_error_t error;
	uint32_t current;

	if(state != i2c_state_start_send)
		return(i2c_error_invalid_state_not_send_start);

	// wait for scl and sda to be released by all masters and slaves
	
	state = i2c_state_bus_wait_1;

	if((error = wait_idle()) != i2c_error_ok)
		return(error);

	// set sda to high
	clear_scl();
	i2c_delay();

	if(scl_is_set())
		return(i2c_error_bus_lock);

	set_sda();
	i2c_delay();

	if(!sda_is_set())
		return(i2c_error_sda_stuck);

	set_scl();
	i2c_delay();

	state = i2c_state_bus_wait_2;

	// demand bus is idle for a minimum window
	for(current = i2c_config_scl_sampling_window; current > 0; current--)
	{
		if(!scl_is_set() || !sda_is_set())
			return(i2c_error_bus_lock);
		short_i2c_delay();
	}

	// generate start condition by leaving scl high and pulling sda low
	clear_sda();
	i2c_delay();

	if(sda_is_set())
		return(i2c_error_sda_stuck);

	return(i2c_error_ok);
}

irom static i2c_error_t send_stop(void)
{
	i2c_error_t error;

	if(state != i2c_state_stop_send)
		return(i2c_error_invalid_state_not_send_stop);

	// at this point scl should be high and sda is unknown
	// wait for scl to be released by all masters and slaves
	if((error = wait_idle()) != i2c_error_ok)
		return(error);

	i2c_delay();

	// set sda to low
	clear_scl();
	i2c_delay();

	if(scl_is_set())
		return(i2c_error_bus_lock);

	clear_sda();
	i2c_delay();

	if(sda_is_set())
		return(i2c_error_sda_stuck);

	set_scl();
	i2c_delay();

	if(sda_is_set())
		return(i2c_error_sda_stuck);

	if(!scl_is_set())
		return(i2c_error_bus_lock);

	// now generate the stop condition by leaving scl high and setting sda high
	set_sda();
	i2c_delay();

	if(!scl_is_set())
		return(i2c_error_bus_lock);

	if(!sda_is_set())
		return(i2c_error_sda_stuck);

	return(i2c_error_ok);
}

irom static i2c_error_t send_bit(bool bit)
{
	i2c_error_t error;

	// at this point scl should be high and sda will be unknown
	// wait for scl to be released by slave (clock stretching)
	if((error = wait_idle()) != i2c_error_ok)
		return(error);
	
	clear_scl();
	i2c_delay();

	if(scl_is_set())
		return(i2c_error_bus_lock);

	if(bit)
	{
		set_sda();
		i2c_delay();

		if(!sda_is_set())
			return(i2c_error_sda_stuck);
	}
	else
	{
		clear_sda();
		i2c_delay();

		if(sda_is_set())
			return(i2c_error_sda_stuck);
	}

	set_scl();
	i2c_delay();

	// take care of clock stretching
	if((error = wait_idle()) != i2c_error_ok)
		return(error);

	return(i2c_error_ok);
}

irom static i2c_error_t send_byte(uint32_t byte)
{
	i2c_error_t error;
	uint32_t current;

	if((state != i2c_state_address_send) && (state != i2c_state_data_send_data))
		return(i2c_error_invalid_state_not_send_address_or_data);

	for(current = 8; current > 0; current--)
	{
		if((error = send_bit(byte & 0x80)) != i2c_error_ok)
			return(error);
		byte <<= 1;
	}

	return(i2c_error_ok);
}

irom static i2c_error_t receive_bit(bool *bit)
{
	uint32_t current;
	uint32_t total;
	i2c_error_t error;

	// at this point scl should be high and sda is unknown,
	// but should be high before reading
	if(state == i2c_state_idle)
		return(i2c_error_invalid_state_idle);

	// wait for scl to be released by slave (clock stretching)
	if((error = wait_idle()) != i2c_error_ok)
		return(error);
	
	// make sure sda is high (open) so slave can pull it
	// do it while clock is pulled
	clear_scl();
	set_sda();

	// wait for slave to pull/release sda
	i2c_delay();

	// release clock again
	set_scl();

	// take care of clock stretching
	if((error = wait_idle()) != i2c_error_ok)
		return(error);

	i2c_delay();

	// do oversampling of sda, to implement a software
	// low-pass filter / spike filter
	for(total = 0, current = 0; current < i2c_config_sda_sampling_window; current++)
	{
		uint32_t set;
		set = sda_is_set();

		total += set ? 4 : 0;

		short_i2c_delay();
	}

	if(total < (i2c_config_sda_sampling_window * 1))		// 0-1/4	=> 0
		*bit = 0;
	else if(total < (i2c_config_sda_sampling_window * 3))	// 1/4-3/4	=> error
		return(i2c_error_receive_error);
	else													// 3/4-1	=> 1
		*bit = 1;

	return(i2c_error_ok);
}

irom static i2c_error_t receive_byte(uint8_t *byte)
{
	uint32_t current;
	bool bit;
	i2c_error_t error;

	for(*byte = 0, current = 8; current > 0; current--)
	{
		*byte <<= 1;

		if((error = receive_bit(&bit)) != i2c_error_ok)
			return(error);

		if(bit)
			*byte |= 0x01;
	}

	return(i2c_error_ok);
}

irom static inline i2c_error_t send_ack(bool ack)
{
	i2c_error_t error;

	if(state != i2c_state_data_receive_ack_send)
		return(i2c_error_invalid_state_not_send_ack);

	if((error = send_bit(!ack)) != i2c_error_ok)
		return(error);

	return(i2c_error_ok);
}

irom static i2c_error_t receive_ack(bool *ack)
{
	i2c_error_t error;
	bool bit;

	if((state != i2c_state_address_ack_receive) && (state != i2c_state_data_send_ack_receive))
		return(i2c_error_invalid_state_not_receive_ack);

	if((error = receive_bit(&bit)) != i2c_error_ok)
		return(error);

	*ack = !bit;

	return(i2c_error_ok);
}

irom static i2c_error_t send_header(uint32_t address, i2c_direction_t direction)
{
	i2c_error_t error;
	bool ack;

	if(state != i2c_state_header_send)
		return(i2c_error_invalid_state_not_send_header);

	address <<= 1;
	address |= direction == i2c_direction_receive ? 0x01 : 0x00;

	state = i2c_state_start_send;

	if((error = send_start()) != i2c_error_ok)
		return(error);

	state = i2c_state_address_send;

	if((error = send_byte(address)) != i2c_error_ok)
		return(error);

	state = i2c_state_address_ack_receive;

	if((error = receive_ack(&ack)) != i2c_error_ok)
		return(error);

	state = i2c_state_address_ack_received;

	if(!ack)
		return(i2c_error_address_nak);

	return(i2c_error_ok);
}

irom i2c_error_t i2c_reset(void)
{
	uint32_t current;

	if(!i2c_flags.init_done)
		return(i2c_error_no_init);

	state = i2c_state_stop_send;

	send_stop();

	// if someone is holding the sda line, simulate clock cycles
	// to make them release it
	for(current = i2c_config_idle_timeout; current > 0; current--)
	{
		if(sda_is_set())
			break;

		clear_scl();
		i2c_delay();
		set_scl();
		i2c_delay();
	}

	if(!sda_is_set())
		return(i2c_error_sda_stuck);

	if(!scl_is_set())
		return(i2c_error_bus_lock);

	state = i2c_state_idle;

	return(i2c_error_ok);
}

irom i2c_error_t i2c_init(uint32_t sda, uint32_t scl, uint32_t delay)
{
	i2c_sda = 1 << sda;
	i2c_scl = 1 << scl;
	transaction_bit_delay = delay;

	i2c_flags.init_done = true;

	return(i2c_reset());
}

irom i2c_error_t i2c_send(uint32_t address, uint32_t length, const uint8_t *bytes)
{
	uint32_t current;
	i2c_error_t error;
	bool ack;

	if(!i2c_flags.init_done)
		return(i2c_error_no_init);

	if(state != i2c_state_idle)
		return(i2c_error_invalid_state_not_idle);

	state = i2c_state_header_send;

	if((error = send_header(address, i2c_direction_send)) != i2c_error_ok)
		return(error);

	for(current = 0; current < length; current++)
	{
		state = i2c_state_data_send_data;

		if((error = send_byte(bytes[current])) != i2c_error_ok)
			return(error);

		state = i2c_state_data_send_ack_receive;

		if((error = receive_ack(&ack)) != i2c_error_ok)
			return(error);

		state = i2c_state_data_send_ack_received;

		if(!ack)
			return(i2c_error_data_nak);
	}

	state = i2c_state_stop_send;

	if((error = send_stop()) != i2c_error_ok)
		return(error);

	state = i2c_state_idle;

	return(i2c_error_ok);
}

irom i2c_error_t i2c_receive(uint32_t address, uint32_t length, uint8_t *bytes)
{
	uint32_t current;
	i2c_error_t error;

	if(!i2c_flags.init_done)
		return(i2c_error_no_init);

	if(state != i2c_state_idle)
		return(i2c_error_invalid_state_not_idle);

	state = i2c_state_header_send;

	if((error = send_header(address, i2c_direction_receive)) != i2c_error_ok)
		return(error);

	for(current = 0; current < length; current++)
	{
		state = i2c_state_data_receive_data;

		if((error = receive_byte(&bytes[current])) != i2c_error_ok)
			return(error);

		state = i2c_state_data_receive_ack_send;

		if((error = send_ack((current + 1) < length)) != i2c_error_ok)
			return(error);
	}

	state = i2c_state_stop_send;

	if((error = send_stop()) != i2c_error_ok)
		return(error);

	state = i2c_state_idle;

	return(i2c_error_ok);
}

irom i2c_error_t i2c_send_1(uint32_t address, uint32_t byte0)
{
	uint8_t bytes[1];

	bytes[0] = byte0;

	return(i2c_send(address, 1, bytes));
}

irom i2c_error_t i2c_send_2(uint32_t address, uint32_t byte0, uint32_t byte1)
{
	uint8_t bytes[2];

	bytes[0] = byte0;
	bytes[1] = byte1;

	return(i2c_send(address, 2, bytes));
}

irom i2c_error_t i2c_send_3(uint32_t address, uint32_t byte0, uint32_t byte1, uint32_t byte2)
{
	uint8_t bytes[3];

	bytes[0] = byte0;
	bytes[1] = byte1;
	bytes[2] = byte2;

	return(i2c_send(address, 3, bytes));
}
