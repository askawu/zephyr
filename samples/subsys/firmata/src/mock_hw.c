/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mock_hw.h"
#include <console.h>

#define TOTAL_PINS 15
#define PORT_PINS 8

struct mock_hw_pin {
	u8_t  mode;
	u8_t  report;
	u16_t value;
};

static struct mock_hw_pin pins[TOTAL_PINS] = {
	/* Pin 0 */
	{ FIRMATA_PIN_MODE_SERIAL, 0, 0 },
	/* Pin 1 */
	{ FIRMATA_PIN_MODE_SERIAL, 0, 0 },
	/* Pin 2 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 3 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 4 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 5 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 6 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 7 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 8 */
	{ FIRMATA_PIN_MODE_INPUT, 0, 0 },
	/* Pin 9 */
	{ FIRMATA_PIN_MODE_ANALOG, 0, 0 },
	/* Pin 10 */
	{ FIRMATA_PIN_MODE_ANALOG, 0, 0 },
	/* Pin 11 */
	{ FIRMATA_PIN_MODE_ANALOG, 0, 0 },
	/* Pin 12 */
	{ FIRMATA_PIN_MODE_ANALOG, 0, 0 },
	/* Pin 13 */
	{ FIRMATA_PIN_MODE_ANALOG, 0, 0 },
	/* Pin 14 */
	{ FIRMATA_PIN_MODE_ANALOG, 0, 0 },
};

static u8_t analog_mapping[] = {
	/* Set to 0x7F if it's not analog input */
	/* pin 0 */
	0x7F,
	/* pin 1 */
	0x7F,
	/* pin 2 */
	0x7F,
	/* pin 3 */
	0x7F,
	/* pin 4 */
	0x7F,
	/* pin 5 */
	0x7F,
	/* pin 6 */
	0x7F,
	/* pin 7 */
	0x7F,
	/* pin 8 */
	0x7F,
	/* pin 9 */
	0x00,
	/* pin 10 */
	0x01,
	/* pin 11 */
	0x02,
	/* pin 12 */
	0x03,
	/* pin 13 */
	0x04,
	/* pin 14 */
	0x05,
};

static struct firmata_pin_cap pin_caps[] = {
        /* Pin 0 */
	FIRMATA_PIN_CAP_STATIC_INIT(0, FIRMATA_PIN_MODE_SERIAL,
				    FIRMATA_PIN_RES_SERIAL_RX0),

        /* Pin 1 */
	FIRMATA_PIN_CAP_STATIC_INIT(1, FIRMATA_PIN_MODE_SERIAL,
				    FIRMATA_PIN_RES_SERIAL_TX0),

        /* Pin 2 */
	FIRMATA_PIN_CAP_STATIC_INIT(2, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(2, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(2, FIRMATA_PIN_MODE_I2C, 1),

        /* Pin 3 */
	FIRMATA_PIN_CAP_STATIC_INIT(3, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(3, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(3, FIRMATA_PIN_MODE_PWM, 10),

        /* Pin 4 */
	FIRMATA_PIN_CAP_STATIC_INIT(4, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(4, FIRMATA_PIN_MODE_OUTPUT, 1),

        /* Pin 5 */
	FIRMATA_PIN_CAP_STATIC_INIT(5, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(5, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(5, FIRMATA_PIN_MODE_PWM, 10),

        /* Pin 6 */
	FIRMATA_PIN_CAP_STATIC_INIT(6, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(6, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(6, FIRMATA_PIN_MODE_PWM, 10),

        /* Pin 7 */
	FIRMATA_PIN_CAP_STATIC_INIT(7, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(7, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(7, FIRMATA_PIN_MODE_I2C, 1),

        /* Pin 8 */
	FIRMATA_PIN_CAP_STATIC_INIT(8, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(8, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(8, FIRMATA_PIN_MODE_I2C, 1),

        /* Pin 9 */
	FIRMATA_PIN_CAP_STATIC_INIT(9, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(9, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(9, FIRMATA_PIN_MODE_ANALOG, 10),

        /* Pin 10 */
	FIRMATA_PIN_CAP_STATIC_INIT(10, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(10, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(10, FIRMATA_PIN_MODE_ANALOG, 10),

        /* Pin 11 */
	FIRMATA_PIN_CAP_STATIC_INIT(11, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(11, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(11, FIRMATA_PIN_MODE_ANALOG, 10),

        /* Pin 12 */
	FIRMATA_PIN_CAP_STATIC_INIT(12, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(12, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(12, FIRMATA_PIN_MODE_ANALOG, 10),

        /* Pin 13 */
	FIRMATA_PIN_CAP_STATIC_INIT(13, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(13, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(13, FIRMATA_PIN_MODE_ANALOG, 10),

        /* Pin 14 */
	FIRMATA_PIN_CAP_STATIC_INIT(14, FIRMATA_PIN_MODE_INPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(14, FIRMATA_PIN_MODE_OUTPUT, 1),
	FIRMATA_PIN_CAP_STATIC_INIT(14, FIRMATA_PIN_MODE_ANALOG, 10),
};

void mock_hw_init(void)
{
	console_init();
}

u8_t mock_hw_serial_input(void)
{
	return console_getchar();
}

int mock_hw_serial_output(struct firmata_context *ctx, const u8_t *buf,
			  u16_t len, void *user_data)
{
	for (u16_t i = 0; i < len; i++) {
		console_putchar(buf[i]);
	}

	return 0;
}

int mock_hw_aio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin, u16_t val,
		   void *user_data)
{
	u16_t copy;

	firmata_output_string_data(ctx, "aio cb\n");

	if (pin >= TOTAL_PINS) {
		return -EINVAL;
	}

	copy = pins[pin].value;
	pins[pin].value = val;

	if (pins[pin].report == FIRMATA_ENABLE_REPORT && copy != val) {
		firmata_output_analog_msg(ctx, pin, val);
	}

	return 0;
}

int mock_hw_dio_cb(struct firmata_context *ctx, u8_t cmd, u8_t port, u16_t val,
		   void *user_data)
{
	u16_t base = port * PORT_PINS;
	u16_t last = port * PORT_PINS + (PORT_PINS - 1);
	u16_t i = base;
	u8_t shift = 0;
	u16_t result = 0;
	bool changed = false;

	firmata_output_string_data(ctx, "dio cb\n");

	if (base >= TOTAL_PINS) {
		return -EINVAL;
	}

	while (i <= last) {
		u16_t new_val;

		if (i >= TOTAL_PINS) {
			break;
		}

		new_val = (val & (0x0001 << shift)) >> shift;
		if (pins[i].value != new_val) {
			changed = true;
		}

		pins[i].value = new_val;
		result = result | (new_val << shift);

		i++;
		shift++;
	}

	if (pins[base].report == FIRMATA_ENABLE_REPORT && changed) {
		firmata_output_digital_msg(ctx, port, val);
	}

	return 0;
}

int mock_hw_report_aio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			  u16_t val, void *user_data)
{
	if (pin >= TOTAL_PINS) {
		return -EINVAL;
	}

	firmata_output_string_data(ctx, "report aio cb\n");

	pins[pin].report = val;

	if (val == FIRMATA_ENABLE_REPORT) {
		firmata_output_analog_msg(ctx, pin, pins[pin].value);
	}

	return 0;
}

int mock_hw_report_dio_cb(struct firmata_context *ctx, u8_t cmd, u8_t port,
			  u16_t val, void *user_data)
{
	u16_t base = port * PORT_PINS;
	u16_t last = port * PORT_PINS + (PORT_PINS - 1);
	u16_t i = base;
	u8_t shift = 0;
	u16_t result = 0;

	firmata_output_string_data(ctx, "report dio cb\n");

	if (base >= TOTAL_PINS) {
		return -EINVAL;
	}

	while (i <= last) {
		if (i >= TOTAL_PINS) {
			break;
		}

		pins[i].report = val;
		result = result | (pins[i].value << shift);

		i++;
		shift++;
	}

	if (val == FIRMATA_ENABLE_REPORT) {
		firmata_output_digital_msg(ctx, port, result);
	}

	return 0;
}

int mock_hw_set_pin_mode_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			    u16_t val, void *user_data)
{
	firmata_output_string_data(ctx, "set pin mode cb\n");

	return 0;
}

int mock_hw_set_pin_value_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			     u16_t val, void *user_data)
{
	firmata_output_string_data(ctx, "set pin value cb\n");

	return 0;
}

int mock_hw_cap_query_cb(struct firmata_context *ctx, u8_t cmd, u8_t id,
			 void *user_data)
{
	firmata_output_string_data(ctx, "cap query cb\n");

	firmata_output_cap_response(ctx, pin_caps, ARRAY_SIZE(pin_caps));

	return 0;
}

int mock_hw_analog_mapping_query_cb(struct firmata_context *ctx, u8_t cmd,
				    u8_t id, void *user_data)
{
	firmata_output_string_data(ctx, "analog mapping query cb\n");

	firmata_output_analog_mapping_response(ctx, analog_mapping,
					       sizeof(analog_mapping));

	return 0;
}
