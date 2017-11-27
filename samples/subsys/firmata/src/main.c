/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr.h>
#include <firmata.h>
#include "mock_hw.h"

struct firmata_callback mock_hw_cbs = {
	.output               = mock_hw_serial_output,
	.aio                  = mock_hw_aio_cb,
	.dio                  = mock_hw_dio_cb,
	.report_aio           = mock_hw_report_aio_cb,
	.report_dio           = mock_hw_report_dio_cb,
	.set_pin_mode         = mock_hw_set_pin_mode_cb,
	.set_pin_value        = mock_hw_set_pin_value_cb,
	.analog_mapping_query = mock_hw_analog_mapping_query_cb,
	.cap_query            = mock_hw_cap_query_cb,
};

void main(void)
{
	struct firmata_context ctx;

	printk("Firmata sample starts...\n");

	mock_hw_init();

	firmata_init(&ctx);
	firmata_attach_callback(&ctx, &mock_hw_cbs);

	while (1) {
		u8_t c = mock_hw_serial_input();

		firmata_input(&ctx, c);
	}
}

