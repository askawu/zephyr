/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __FIRMATA_H
#define __FIRMATA_H

#include <zephyr/types.h>

/** Firmata protocol version */
#define FIRMATA_PROTOCOL_MAJOR_VERSION 2
#define FIRMATA_PROTOCOL_MINOR_VERSION 6

/** Firmata protocol comamnd */
#define FIRMATA_CMD_ANALOG           0xE0
#define FIRMATA_CMD_DIGITAL          0x90
#define FIRMATA_CMD_REPORT_ANALOG    0xC0
#define FIRMATA_CMD_REPORT_DIGITAL   0xD0
#define FIRMATA_CMD_SYSEX_START      0xF0
#define FIRMATA_CMD_SET_PIN_MODE     0xF4
#define FIRMATA_CMD_SET_PIN_VALUE    0xF5
#define FIRMATA_CMD_SYSEX_END        0xF7
#define FIRMATA_CMD_PROTOCOL_VERSION 0xF9
#define FIRMATA_CMD_SYSTEM_RESET     0xFF

/** Firmata sysex command */
#define FIRMATA_SYSEX_EXTENDED_ID             0x00
#define FIRMATA_SYSEX_ANALOG_MAPPING_QUERY    0x69
#define FIRMATA_SYSEX_ANALOG_MAPPING_RESPONSE 0x6A
#define FIRMATA_SYSEX_CAPABILITY_QUERY        0x6B
#define FIRMATA_SYSEX_CAPABILITY_RESPONSE     0x6C
#define FIRMATA_SYSEX_PIN_STATE_QUERY         0x6D
#define FIRMATA_SYSEX_PIN_STATE_RESPONSE      0x6E
#define FIRMATA_SYSEX_EXTENDED_ANALOG         0x6F
#define FIRMATA_SYSEX_STRING_DATA             0x71
#define FIRMATA_SYSEX_REORT_FIRMWARE          0x79
#define FIRMATA_SYSEX_SAMPLING_INTERVAL       0x7A
#define FIRMATA_SYSEX_NON_REALTIME            0x7E
#define FIRMATA_SYSEX_REALTIME                0x7F

/** Firmata pin mode */
/** Digital input */
#define FIRMATA_PIN_MODE_INPUT          0x00
/** Digital output */
#define FIRMATA_PIN_MODE_OUTPUT         0x01
/** Analog input */
#define FIRMATA_PIN_MODE_ANALOG         0x02
#define FIRMATA_PIN_MODE_PWM            0x03
#define FIRMATA_PIN_MODE_SERVO          0x04
#define FIRMATA_PIN_MODE_SHIFT          0x05
#define FIRMATA_PIN_MODE_I2C            0x06
#define FIRMATA_PIN_MODE_ONEWIRE        0x07
#define FIRMATA_PIN_MODE_STEPPER        0x08
#define FIRMATA_PIN_MODE_ENCODER        0x09
#define FIRMATA_PIN_MODE_SERIAL         0x0A
#define FIRMATA_PIN_MODE_PULLUP         0x0B

/** Firmata pin resolution */
#define FIRMATA_PIN_RES_SERIAL_RX0 0x00
#define FIRMATA_PIN_RES_SERIAL_TX0 0x01
#define FIRMATA_PIN_RES_SERIAL_RX1 0x02
#define FIRMATA_PIN_RES_SERIAL_TX1 0x03
#define FIRMATA_PIN_RES_SERIAL_RX2 0x04
#define FIRMATA_PIN_RES_SERIAL_TX2 0x05
#define FIRMATA_PIN_RES_SERIAL_RX3 0x06
#define FIRMATA_PIN_RES_SERIAL_TX3 0x07

#define FIRMATA_DISABLE_REPORT 0x00
#define FIRMATA_ENABLE_REPORT  0x01

struct firmata_context;

/**
 * @typedef firmata_output_cb_t
 * @brief Firmata output callback.
 *
 * @details The firmata output callback is introduced to decouple the data
 * transmission from the firmata protocol. The callback is responsible for
 * conveying the message to the client in the desired way. By using this
 * callback, the firmata protocol can be realized over various interfaces, such
 * as serial port, ethernet, or bluetooth.
 *
 * @param ctx The address of the firmata context.
 * @param buf The buffer of the message. Note that the buffer will be released
 * immediatedly after this callback returns;
 * @param len The length of the buffer.
 * @param user_data The user data given in firmata context.
 *
 * @return 0 if ok, <0 if error.
 */
typedef int (*firmata_output_cb_t)(struct firmata_context *ctx,
				   const u8_t *buf,
				   u16_t len,
				   void *user_data);

/**
 * @typedef firmata_io_cb_t
 * @brief Firmata analog/digital operation callback.
 *
 * @details This is a generic callback which is called when the client
 * performs a specified command with the specified channel and value.
 *
 * @param ctx The address of the firmata context.
 * @param cmd The command from the client.
 * @param channel The channel number of the command. For example,
 * it can be the analog pin number or the digitial port number.
 * @param val The value of the command.
 * @param user_data The user data given in firmata context.
 *
 * @return 0 if ok, <0 if error.
 */
typedef int (*firmata_io_cb_t)(struct firmata_context *ctx,
			       u8_t cmd,
			       u8_t channel,
			       u16_t val,
			       void *user_data);

/**
 * @typedef firmata_cmd_cb_t
 * @brief Firmata command callback.
 *
 * @details This is a generic callback which is called when the client
 * performs a specified command, such as system reset and sysex commands.
 *
 * @param ctx The address of the firmata context.
 * @param cmd The command from the client.
 * @param id The feature ID of sysex command.
 * If cmd is not FIRMATA_CMD_SYSEX_START, id is not used and should be 0.
 * @param user_data The user data given in firmata context.
 *
 * @return 0 if ok, <0 if error.
 */
typedef int (*firmata_cmd_cb_t)(struct firmata_context *ctx,
				u8_t cmd,
				u8_t id,
				void *user_data);

struct firmata_callback {

	/** The callback called to send the message to the client. */
	firmata_output_cb_t output;

	/** The callback called when "analog I/O message (0xE0)" is received */
	firmata_io_cb_t aio;

	/** The callback called when "digital I/O message (0x90)" is received */
	firmata_io_cb_t dio;

	/** The callback called when "report analog pin (0xC0)" is received */
	firmata_io_cb_t report_aio;

	/** The callback called when "report digital pin (0xD0)" is received */
	firmata_io_cb_t report_dio;

	/** The callback called when "set pin mode (0xF4)" is received */
	firmata_io_cb_t set_pin_mode;

	/** The callback called when "set pin value (0xF5)" is received */
	firmata_io_cb_t set_pin_value;

	/** The callback called when "analog mapping query (0xF0 0x69)" is
	 *  received.
	 */
	firmata_cmd_cb_t analog_mapping_query;

	/** The callback called when "capability query (0xF0 0x6B)" is
	 *  received.
	 */
	firmata_cmd_cb_t cap_query;
};

/** Firmata context */
struct firmata_context {

	/* Various callbacks */
	struct firmata_callback cbs;

	/** The user data of the firmata callbacks */
	void *cb_user_data;

	/** Internal use only
	 *  The write index of the input buffer.
	 */
	u16_t input_pos;

	/** Internal use only
	 *  The write index of the output buffer.
	 */
	u16_t output_pos;

	/** Internal use only
	 *  The remain data count of the current command.
	 */
	u16_t remain_cnt;

	/** Internal use only
	 *  The state of firmata state machine.
	 */
	u8_t  state;

	/** Internal use only
	 *  The buffer used to hold the incoming message.
	 */
	u8_t input_buf[CONFIG_FIRMATA_INPUT_BUF_SIZE];

	/** Internal use only
	 *  The buffer used to hold the outgoing message
	 */
	u8_t output_buf[CONFIG_FIRMATA_OUTPUT_BUF_SIZE];
};

/** The pin capability structure */
struct firmata_pin_cap {

	/** The pin number */
	u8_t pin_num;

	/** The supported mode. Should be one of FIRMATA_PIN_MODE macros. */
	u8_t mode;

	/** The mode resolution.
	 *  If the mode is FIRMATA_PIN_MODE_SERIAL, it should be one of
	 *  FIRMATA_PIN_RES_SERIAL macros. Otherwise, it should represent
	 *  the resolution of the pin, such as 10 for 10-bit an analog pin or 1
	 *  for a digital pin.
	 *
	 *  See https://github.com/firmata/protocol/blob/master/protocol.md
	 */
	u8_t resolution;
};

/* A helper macro to reduce the duplication of firmata pin capability
 * initialization.
 */
#define FIRMATA_PIN_CAP_STATIC_INIT(pin, m, res) \
	{ \
		.pin_num = pin, \
		.mode = m, \
		.resolution = res, \
	}


/**
 * @brief Initialize firmata context.
 *
 * @param ctx The address of the firmata context.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_init(struct firmata_context *ctx);

/**
 * @brief Input one byte data to firmata context.
 *
 * @details This function is called when new data is received from the client.
 *
 * @param ctx The address of the firmata context.
 * @param data One byte data.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_input(struct firmata_context *ctx, u8_t data);

/**
 * @brief Input variable-length data to firmata context.
 *
 * @details This function is a variation of firmata_input(). This function is
 * called when variable-length data is received from the client.
 *
 * @param ctx The address of the firmata context.
 * @param buf The address of the variable-length buffer.
 * @param len The buffer size.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_input_buf(struct firmata_context *ctx, u8_t *buf, u16_t len);

/**
 * @brief Output variable-length data to the client.
 *
 * @details This function is called when the caller wants to send a message to
 * the client. The caller should be responsible for the message format. At end
 * of this function, the message will be sent to the client by the attached
 * output callback. See firmata_output_cb_t for more info.
 *
 * @param ctx The address of the firmata context.
 * @param buf The address of the variable-length buffer.
 * @param len The buffer size.
 *
 * @return 0 if ok, <0 if error.
 */
void firmata_output(struct firmata_context *ctx, u8_t *buf, u16_t len);

/**
 * @brief Output an analog message to the client.
 *
 * @details This function is called when the caller wants to send an analog
 * message to the client. Usually, it's called when the caller wants to report
 * the new value of an analog pin. At the end of this function, the message will
 * be sent to the client by the attached output callback. See
 * firmata_output_cb_t for more info.
 *
 * @param ctx The address of the firmata context.
 * @param pin The pin number of the analog pin.
 * @param val The value of the analog pin.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_output_analog_msg(struct firmata_context *ctx, u8_t pin, u16_t val);

/**
 * @brief Output a digital message to the client.
 *
 * @details This function is called when the caller wants to send a digital
 * message to the client. Usually, it's called when the caller wants to report
 * the new value of an digital port. At the end of this function, the message
 * will be sent to the client by the attached output callback. See
 * firmata_output_cb_t for more info.
 *
 * @param ctx The address of the firmata context.
 * @param pin The port number of the digital port.
 * @param val The value of the digital port.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_output_digital_msg(struct firmata_context *ctx, u8_t port,
			       u16_t val);

/**
 * @brief Output a string data to the client.
 *
 * @details Firmata allows string message between the server and client. This
 * function is called when the caller wants to send a string data to the client.
 * At the end of this function, the message will be sent to the client by the
 * attached output callback. See firmata_output_cb_t for more info.
 *
 * @param ctx The address of the firmata context.
 * @param string The NULL-terminated string.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_output_string_data(struct firmata_context *ctx, const char *string);

/**
 * @brief Output the capability response to the client.
 *
 * @details Firmata allows the client to query the capability of pins by
 * capability query command. Usaully, this function is called when the caller
 * wants to send the capability response to answer the capability query. At the
 * end of this function, the message will be sent to the client by the attached
 * output callback. See firmata_output_cb_t for more info.
 *
 * @param ctx The address of the firmata context.
 * @param caps The array of pin capabilities.
 * @param num_caps The array size.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_output_cap_response(struct firmata_context *ctx,
				struct firmata_pin_cap *caps,
				u16_t num_caps);

/**
 * @brief Output the analog mapping response to the client.
 *
 * @details According to firmata protocol, the normal pin number and ananlog pin
 * number are different. For example, the pin #9 can be mapped to analog pin #0.
 * The analog mapping is a mapping table that maps the normal pin number to
 * the analog pin number. Firmata allows the client to query the analog mapping.
 * With the analog mapping response, the client can get the total number of pins
 * and which pins are used as analog pins. Some client libraries, such as
 * PyMata, send the query and wait for the response at the beginning, terminate
 * if no resonse after timeout. This function is called when the caller wants to
 * send the analog mapping response to answer the analog mapping query.
 *
 * @param ctx The address of the firmata context.
 * @param map The byte array represents the mapping. For example, map[0] is set
 * to 0x7F if pin #0 is not an analog input pin and map[9] is set to 0 if pin #9
 * is ananlog pin #0.
 * @param map_size The array size. Should equal to the total number of pins.
 *
 * @return 0 if ok, <0 if error.
 */
int firmata_output_analog_mapping_response(struct firmata_context *ctx,
					   u8_t *map,
					   u16_t map_size);


/* TODO: desc */
int firmata_attach_callback(struct firmata_context *ctx,
			    struct firmata_callback *cbs);

#endif
