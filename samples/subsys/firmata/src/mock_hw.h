/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MOCK_HW_H__
#define __MOCK_HW_H__

#include <zephyr.h>
#include <firmata.h>

void mock_hw_init(void);

u8_t mock_hw_serial_input(void);

int mock_hw_serial_output(struct firmata_context *ctx, const u8_t *buf,
			  u16_t len, void *user_data);

int mock_hw_aio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin, u16_t val,
		   void *user_data);

int mock_hw_dio_cb(struct firmata_context *ctx, u8_t cmd, u8_t port, u16_t val,
		   void *user_data);

int mock_hw_report_aio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			  u16_t val, void *user_data);

int mock_hw_report_dio_cb(struct firmata_context *ctx, u8_t cmd, u8_t port,
			  u16_t val, void *user_data);

int mock_hw_set_pin_mode_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			    u16_t val, void *user_data);

int mock_hw_set_pin_value_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			     u16_t val, void *user_data);

int mock_hw_set_pin_value_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
			     u16_t val, void *user_data);

int mock_hw_cap_query_cb(struct firmata_context *ctx, u8_t cmd, u8_t sub_cmd,
			 void *user_data);

int mock_hw_analog_mapping_query_cb(struct firmata_context *ctx, u8_t cmd,
				    u8_t sub_cmd, void *user_data);

#endif /* __MOCK_HW_H */
