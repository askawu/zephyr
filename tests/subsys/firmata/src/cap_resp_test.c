#include <zephyr/types.h>
#include <ztest.h>
#include <firmata.h>

static struct firmata_pin_cap pin_caps[] = {
	/* Pin 0 */
	{
		.pin_num = 0,
		.mode = FIRMATA_PIN_MODE_SERIAL,
		.resolution = FIRMATA_PIN_RES_SERIAL_RX0,
	},
	{
		.pin_num = 0,
		.mode = FIRMATA_PIN_MODE_INPUT,
		.resolution = 1,
	},
	{
		.pin_num = 0,
		.mode = FIRMATA_PIN_MODE_OUTPUT,
		.resolution = 1,
	},
	{
		.pin_num = 0,
		.mode = FIRMATA_PIN_MODE_ANALOG,
		.resolution = 10,
	},
	/* Pin 1 */
	{
		.pin_num = 1,
		.mode = FIRMATA_PIN_MODE_SERIAL,
		.resolution = FIRMATA_PIN_RES_SERIAL_TX0,
	},
	{
		.pin_num = 1,
		.mode = FIRMATA_PIN_MODE_INPUT,
		.resolution = 1,
	},
	{
		.pin_num = 1,
		.mode = FIRMATA_PIN_MODE_OUTPUT,
		.resolution = 1,
	},
	{
		.pin_num = 1,
		.mode = FIRMATA_PIN_MODE_ANALOG,
		.resolution = 10,
	},
	/* Pin 2 */
	{
		.pin_num = 2,
		.mode = FIRMATA_PIN_MODE_INPUT,
		.resolution = 1,
	},
	{
		.pin_num = 2,
		.mode = FIRMATA_PIN_MODE_OUTPUT,
		.resolution = 1,
	},
	{
		.pin_num = 2,
		.mode = FIRMATA_PIN_MODE_I2C,
		.resolution = 1,
	},
	/* Pin 3 */
	{
		.pin_num = 3,
		.mode = FIRMATA_PIN_MODE_INPUT,
		.resolution = 1,
	},
	{
		.pin_num = 3,
		.mode = FIRMATA_PIN_MODE_OUTPUT,
		.resolution = 1,
	},
	{
		.pin_num = 3,
		.mode = FIRMATA_PIN_MODE_I2C,
		.resolution = 1,
	},
};

static u8_t resp_msg[] = { /* HEADER */
			   0xF0, 0x6C,
			   /* PIN 0 */
			   0x0A, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x0A, 0x7F,
			   /* PIN 1 */
			   0x0A, 0x01, 0x00, 0x01, 0x01, 0x01, 0x02, 0x0A, 0x7F,
			   /* PIN 2 */
			   0x00, 0x01, 0x01, 0x01, 0x06, 0x01, 0x7F,
			   /* PIN 3 */
			   0x00, 0x01, 0x01, 0x01, 0x06, 0x01, 0x7F,
			   /* FOOTER */
			   0xF7 };

static int resp_msg_index = 0;

static int test_output_cb(struct firmata_context *ctx, const u8_t *buf,
			  u16_t len, void *user_data)
{
	int index = 0;

	zassert_true(resp_msg_index < sizeof(resp_msg), "size error");

	while (index < len) {
		zassert_equal(resp_msg[resp_msg_index], buf[index],
			      "msg error");
		index++;
		resp_msg_index++;
	}

	return 0;
}

void test_output_cap_response(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs = { 0 };
	int ret;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.output = test_output_cb;
	firmata_attach_callback(&ctx, &cbs);
	firmata_output_cap_response(&ctx, pin_caps, ARRAY_SIZE(pin_caps));
}

