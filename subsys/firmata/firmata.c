/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define SYS_LOG_DOMAIN "firmata"
#include <logging/sys_log.h>

#include <zephyr/types.h>
#include <errno.h>
#include <string.h>
#include <firmata.h>

#define MD_LSB_MASK 0x7f
#define MD_MSB_MASK 0x80
#define MD_MSB_SHIFT 7

#define U16_MD_MAX 0x3FFF

#define CMD_MASK 0x80
#define CHANNEL_MASK 0x0F

#define HIGH_NIBBLE_EQ(v, m) ((v & 0xF0) == m)
#define FIRST_BYTE 1
#define SECOND_BYTE 2

#define ANALOG_MSG_SIZE 3
#define DIGITAL_MSG_SIZE 3
#define CAPABILITY_QUERY_MSG_SIZE 3
#define ANALOG_MAPPING_QUERY_MSG_SIZE 3

#define CAPABILITY_SEPARATOR 0x7F

#define CALL_CB(ctx, cb, ...)                           \
	do {                                            \
		if (ctx->cbs.cb) {                      \
			ctx->cbs.cb(ctx,                \
				    __VA_ARGS__,        \
				    ctx->cb_user_data); \
		}                                       \
	} while (0)

/* Build-time check of firmata output buffer size */
#if CONFIG_FIRMATA_OUTPUT_BUF_SIZE < 2
#error the minimum output buffer size should be 2
#endif

enum firmata_state {
	FIRMATA_STATE_CMD,
	FIRMATA_STATE_CMD_DATA,
	FIRMATA_STATE_READY,
	FIRMATA_STATE_MAX,
};

int midi_to_u16(u8_t *buf, u16_t len, u16_t offset, u16_t *val)
{
	u8_t lsb = 0;
	u8_t msb = 0;

	if (!buf || !val) {
		return -EFAULT;
	}

	if (len <= 0 || offset >= len || offset + 1 >= len) {
		return -ENOBUFS;
	}

	lsb = *(buf + offset);
	msb = *(buf + offset + 1);
	*val = (msb << MD_MSB_SHIFT) | lsb;

	return 0;
}

int u16_to_midi(u8_t *buf, u16_t len, u16_t offset, u16_t val)
{
	if (!buf) {
		return -EFAULT;
	}

	if (len <= 0 || offset >= len || offset + 1 >= len) {
		return -ENOBUFS;
	}

	*(buf + offset) = (u8_t)(val & MD_LSB_MASK);
	*(buf + offset + 1) = (u8_t)(val >> MD_MSB_SHIFT);

	return 0;
}

static int reset_state(struct firmata_context *ctx)
{
	ctx->state = FIRMATA_STATE_CMD;
	ctx->remain_cnt = 0;
	ctx->input_pos = 0;
	memset(ctx->input_buf, 0, CONFIG_FIRMATA_INPUT_BUF_SIZE);

	return 0;
}

/* Input buffer helper functions */

static int get_input_buf_size(struct firmata_context *ctx)
{
	if (ctx->input_pos > 0) {
		return ctx->input_pos;
	}

	return 0;
}

static int access_input_buf(struct firmata_context *ctx, u16_t index,
			    u8_t *data)
{
	if (index >= ctx->input_pos) {
		return -EINVAL;
	}

	*data = ctx->input_buf[index];

	return 0;
}

static int append_input_buf(struct firmata_context *ctx, u8_t data)
{
	if (ctx->input_pos == CONFIG_FIRMATA_INPUT_BUF_SIZE) {
		return -ENOBUFS;
	}

	ctx->input_buf[ctx->input_pos] = data;
	ctx->input_pos++;

	return 0;
}

/* Output buffer helper functions */

static int flush_output_buf(struct firmata_context *ctx)
{
	firmata_output(ctx, ctx->output_buf, ctx->output_pos);
	ctx->output_pos = 0;

	return 0;
}

static int append_output_buf(struct firmata_context *ctx, u8_t data)
{
	if (ctx->output_pos == CONFIG_FIRMATA_OUTPUT_BUF_SIZE) {
		flush_output_buf(ctx);
	}

	ctx->output_buf[ctx->output_pos] = data;
	ctx->output_pos += 1;

	return 0;
}

static int validate_cmd(struct firmata_context *ctx, u8_t cmd)
{
	if (!(cmd & CMD_MASK)) {
		return -ENOTSUP;
	}

	if (HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_ANALOG) ||
	    HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_DIGITAL) ||
	    cmd == FIRMATA_CMD_SET_PIN_MODE ||
	    cmd == FIRMATA_CMD_SET_PIN_VALUE) {
		ctx->remain_cnt = 2;
		ctx->state = FIRMATA_STATE_CMD_DATA;
	} else if (HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_REPORT_ANALOG) ||
		   HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_REPORT_DIGITAL)) {
		ctx->remain_cnt = 1;
		ctx->state = FIRMATA_STATE_CMD_DATA;
	} else if (cmd == FIRMATA_CMD_PROTOCOL_VERSION ||
		   cmd == FIRMATA_CMD_SYSTEM_RESET) {
		ctx->remain_cnt = 0;
		ctx->state = FIRMATA_STATE_READY;
	} else if (cmd == FIRMATA_CMD_SYSEX_START) {
		ctx->remain_cnt = 0;
		ctx->state = FIRMATA_STATE_CMD_DATA;
	} else {
		return -ENOTSUP;
	}

	SYS_LOG_DBG("Valid cmd: %x, remain_cnt: %d", cmd, ctx->remain_cnt);

	return append_input_buf(ctx, cmd);
}

static int validate_data(struct firmata_context *ctx, u8_t data)
{
	int ret;
	u8_t cmd;

	ret = access_input_buf(ctx, 0, &cmd);
	if (ret < 0) {
		SYS_LOG_ERR("Failed to access buf: %d", ret);
		return ret;
	}

	if (cmd == FIRMATA_CMD_SYSEX_START && data == FIRMATA_CMD_SYSEX_END) {
		/* sysex end is received, set to ready for further handling */
		ctx->state = FIRMATA_STATE_READY;
		append_input_buf(ctx, data);
		SYS_LOG_DBG("Valid cmd: 0xf7");
		return 0;
	}

	/* if it's not sysex, it should be in data format */
	if (data & CMD_MASK) {
		return -ENOTSUP;
	}

	ret = append_input_buf(ctx, data);
	if (ret < 0) {
		return ret;
	}

	if (ctx->remain_cnt > 0) {
		ctx->remain_cnt--;
		if (ctx->remain_cnt == 0) {
			ctx->state = FIRMATA_STATE_READY;
		}
	}

	SYS_LOG_DBG("Valid data: %x, remain: %d", data, ctx->remain_cnt);

	return 0;
}

static int process_sysex_msg(struct firmata_context *ctx)
{
	int ret = 0;
	u8_t cmd;
	u8_t id;

	ret = access_input_buf(ctx, 0, &cmd);
	if (ret < 0) {
		SYS_LOG_ERR("Failed to access buf: %d", ret);
		return ret;
	}

	ret = access_input_buf(ctx, 1, &id);
	if (ret < 0) {
		SYS_LOG_ERR("Failed to access buf: %d", ret);
		return ret;
	}

	switch (id) {
	case FIRMATA_SYSEX_ANALOG_MAPPING_QUERY:
		if (get_input_buf_size(ctx) != ANALOG_MAPPING_QUERY_MSG_SIZE) {
			return -EINVAL;
		}
		CALL_CB(ctx, analog_mapping_query, cmd, id);
		break;
	case FIRMATA_SYSEX_CAPABILITY_QUERY:
		if (get_input_buf_size(ctx) != CAPABILITY_QUERY_MSG_SIZE) {
			return -EINVAL;
		}
		CALL_CB(ctx, cap_query, cmd, id);
		break;
	default:
		ret = -ENOTSUP;
		break;
	}

	return ret;
}

static int process_msg(struct firmata_context *ctx)
{
	int ret = 0;
	u8_t cmd;

	ret = access_input_buf(ctx, 0, &cmd);
	if (ret < 0) {
		SYS_LOG_ERR("Failed to access buf: %d", ret);
		return ret;
	}

	if (HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_ANALOG) ||
	    HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_DIGITAL)) {
		u8_t ch;
		u16_t val;

		ch = cmd & CHANNEL_MASK;
		ret = midi_to_u16(ctx->input_buf, ctx->input_pos, 1, &val);
		if (ret < 0) {
			SYS_LOG_ERR("Failed to decode md: %d", ret);
			goto clean_up;
		}

		if (HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_ANALOG)) {
			CALL_CB(ctx, aio, cmd, ch, val);
		} else {
			CALL_CB(ctx, dio, cmd, ch, val);
		}

	} else if (HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_REPORT_ANALOG) ||
		   HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_REPORT_DIGITAL)) {
		u8_t ch, val;

		ch = cmd & CHANNEL_MASK;
		ret = access_input_buf(ctx, FIRST_BYTE, &val);
		if (ret < 0) {
			SYS_LOG_ERR("Failed to access buf: %d", ret);
			goto clean_up;
		}

		if (HIGH_NIBBLE_EQ(cmd, FIRMATA_CMD_REPORT_ANALOG)) {
			CALL_CB(ctx, report_aio, cmd, ch, (u16_t)val);
		} else {
			CALL_CB(ctx, report_dio, cmd, ch, (u16_t)val);
		}

	} else if (cmd == FIRMATA_CMD_SET_PIN_MODE ||
		   cmd == FIRMATA_CMD_SET_PIN_VALUE) {
		u8_t pin, val;

		ret = access_input_buf(ctx, FIRST_BYTE, &pin);
		if (ret < 0) {
			SYS_LOG_ERR("Failed to access buf: %d", ret);
			goto clean_up;
		}

		ret = access_input_buf(ctx, SECOND_BYTE, &val);
		if (ret < 0) {
			SYS_LOG_ERR("Failed to access buf: %d", ret);
			goto clean_up;
		}

		if (cmd == FIRMATA_CMD_SET_PIN_MODE) {
			CALL_CB(ctx, set_pin_mode, cmd, pin, (u16_t)val);
		} else {
			CALL_CB(ctx, set_pin_value, cmd, pin, (u16_t)val);
		}

	} else if (cmd == FIRMATA_CMD_PROTOCOL_VERSION) {
		u8_t buf[3];

		buf[0] = FIRMATA_CMD_PROTOCOL_VERSION;
		buf[1] = FIRMATA_PROTOCOL_MAJOR_VERSION;
		buf[2] = FIRMATA_PROTOCOL_MINOR_VERSION;

		firmata_output(ctx, buf, sizeof(buf));

	} else if (cmd == FIRMATA_CMD_SYSEX_START) {
		ret = process_sysex_msg(ctx);
		if (ret < 0) {
			SYS_LOG_ERR("Failed to process sysex msg: %d", ret);
			goto clean_up;
		}
	} else {
		ret = -ENOTSUP;
	}

clean_up:
	reset_state(ctx);

	return ret;
}

int firmata_init(struct firmata_context *ctx)
{
	if (!ctx) {
		return -EFAULT;
	}

	memset(ctx, 0, sizeof(struct firmata_context));

	return 0;
}

int firmata_input(struct firmata_context *ctx, u8_t data)
{
	int ret = 0;

	if (!ctx) {
		return -EFAULT;
	}

	/* Validation */
	if (ctx->state == FIRMATA_STATE_CMD) {
		ret = validate_cmd(ctx, data);
	} else if (ctx->state == FIRMATA_STATE_CMD_DATA) {
		ret = validate_data(ctx, data);
	} else {
		ret = -ENOTSUP;
	}

	if (ret < 0) {
		SYS_LOG_ERR("input error: %u, %x", ctx->state, data);
		reset_state(ctx);
		return ret;
	}

	/* Process */
	if (ctx->state == FIRMATA_STATE_READY) {
		ret = process_msg(ctx);
	}

	return ret;
}

int firmata_input_buf(struct firmata_context *ctx, u8_t *buf, u16_t len)
{
	if (!buf) {
		return -EFAULT;
	}

	for (int i = 0; i < len; i++) {
		int ret;

		ret = firmata_input(ctx, buf[i]);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

void firmata_output(struct firmata_context *ctx, u8_t *buf, u16_t len)
{
	if (!ctx || !buf) {
		return;
	}

	CALL_CB(ctx, output, buf, len);
}

int firmata_output_analog_msg(struct firmata_context *ctx, u8_t pin, u16_t val)
{
	int ret;
	u8_t buf[2];

	if (!ctx) {
		return -EFAULT;
	}

	if (pin > 0x0F || val > U16_MD_MAX) {
		return -EINVAL;
	}

	ret = u16_to_midi(buf, sizeof(buf), 0, val);
	if (ret < 0) {
		return ret;
	}

	append_output_buf(ctx, FIRMATA_CMD_ANALOG | pin);
	append_output_buf(ctx, buf[0]);
	append_output_buf(ctx, buf[1]);

	flush_output_buf(ctx);

	return 0;
}

int firmata_output_digital_msg(struct firmata_context *ctx, u8_t port,
			       u16_t val)
{
	int ret;
	u8_t buf[2];

	if (!ctx) {
		return -EFAULT;
	}

	if (port > 0x0F || val > U16_MD_MAX) {
		return -EINVAL;
	}

	ret = u16_to_midi(buf, sizeof(buf), 0, val);
	if (ret < 0) {
		return ret;
	}

	append_output_buf(ctx, FIRMATA_CMD_DIGITAL | port);
	append_output_buf(ctx, buf[0]);
	append_output_buf(ctx, buf[1]);

	flush_output_buf(ctx);

	return 0;
}

int firmata_output_string_data(struct firmata_context *ctx, const char *string)
{
	u16_t len;

	if (!ctx || !string) {
		return -EFAULT;
	}

	len = strlen(string);
	if (len == 0) {
		return -EINVAL;
	}

	/* Msg: START_SYSEX + STRING_DATA_ID + DATA + END_SYSEX
	 * START_SYSEX:    1 byte (0xF0)
	 * STRING_DATA_ID: 1 byte (0x71)
	 * DATA:           2 * (number of char) bytes
	 * END_SYSEX:      1 byte (0xF7)
	 */
	append_output_buf(ctx, FIRMATA_CMD_SYSEX_START);
	append_output_buf(ctx, FIRMATA_SYSEX_STRING_DATA);

	for (int i = 0; i < len; i++) {
		u8_t msg[2];

		u16_to_midi(msg, sizeof(msg), 0, (u16_t)string[i]);
		append_output_buf(ctx, msg[0]);
		append_output_buf(ctx, msg[1]);
	}

	append_output_buf(ctx, FIRMATA_CMD_SYSEX_END);

	flush_output_buf(ctx);

	return 0;
}

int firmata_output_cap_response(struct firmata_context *ctx,
                                struct firmata_pin_cap *caps,
                                u16_t num_caps)
{
	u8_t curr_pin;

	if (!ctx || !caps) {
		return -EFAULT;
	}

	if (num_caps == 0) {
		return -EINVAL;
	}

	/* Msg: HEADER + TOTAL_PIN * CAP + FOOTER
	 * HEADER: 2 bytes (START_SYSEX + SYSEX_CAPABILITY_RESPONSE)
	 * CAP:    N bytes (MODE + RESOLUTION + SEPARATOR (optional))
	 * FOOTER: 1 byte (END_SYSEX)
	 */

	/* Header */
	append_output_buf(ctx, FIRMATA_CMD_SYSEX_START);
	append_output_buf(ctx, FIRMATA_SYSEX_CAPABILITY_RESPONSE);

	curr_pin = caps[0].pin_num;
	for (int i = 0; i < num_caps; i++) {
		if (curr_pin != caps[i].pin_num) {
			/* Pin num is changed, insert a separator */
			append_output_buf(ctx, CAPABILITY_SEPARATOR);
			curr_pin = caps[i].pin_num;
		}

		append_output_buf(ctx, caps[i].mode);
		append_output_buf(ctx, caps[i].resolution);
	}

	/* Last separator */
	append_output_buf(ctx, CAPABILITY_SEPARATOR);

	/* Footer */
	append_output_buf(ctx, FIRMATA_CMD_SYSEX_END);

	flush_output_buf(ctx);

	return 0;
}

int firmata_output_analog_mapping_response(struct firmata_context *ctx,
					   u8_t *map,
					   u16_t map_size)
{
	if (!ctx || !map) {
		return -EFAULT;
	}

	if (map_size == 0) {
		return -EINVAL;
	}

	/* Msg: HEADER + (TOTAL_PIN * MAP) + FOOTER
	 * HEADER: 2 bytes (START_SYSEX + SYSEX_ANALOG_MAPPING_RESPONSE)
	 * MAP:    1 byte (analog pin # or 0x7F)
	 * FOOTER: 1 byte (END_SYSEX)
	 */

	/* Header */
	append_output_buf(ctx, FIRMATA_CMD_SYSEX_START);
	append_output_buf(ctx, FIRMATA_SYSEX_ANALOG_MAPPING_RESPONSE);

	for (int i = 0; i < map_size; i++) {
		if (map[i] != 0x7F && map[i] > 0x0F) {
			return -EINVAL;
		}

		append_output_buf(ctx, map[i]);
	}

	/* Footer */
	append_output_buf(ctx, FIRMATA_CMD_SYSEX_END);

	flush_output_buf(ctx);

	return 0;
}

int firmata_attach_callback(struct firmata_context *ctx,
		struct firmata_callback *cbs)
{
	if (!ctx || !cbs) {
		return -EFAULT;
	}

	/* Copy callbacks to context cbs */
	memcpy(&ctx->cbs, cbs, sizeof(struct firmata_callback));

	return 0;
}
