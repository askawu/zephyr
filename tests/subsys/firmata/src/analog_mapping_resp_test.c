
#include <zephyr/types.h>
#include <ztest.h>
#include <firmata.h>

static u8_t map[15] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
			0x7F, 0x7F, 0x7F, 0x7F, 0x00,
			0x01, 0x02, 0x03, 0x04, 0x05 };

static u8_t msg[18] = { 0xF0, 0x6A,
			0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
			0x7F, 0x7F, 0x7F, 0x7F, 0x00,
			0x01, 0x02, 0x03, 0x04, 0x05,
			0xF7 };

static u8_t msg_index = 0;

static int test_output_cb(struct firmata_context *ctx, const u8_t *buf,
			  u16_t len, void *user_data)
{
	zassert_equal(0, memcmp(msg + msg_index, buf, len), "data error");

	msg_index += len;

	return 0;
}

void test_output_analog_mapping_response(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs = { 0 };
	int ret;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.output = test_output_cb;
	firmata_attach_callback(&ctx, &cbs);
	firmata_output_analog_mapping_response(&ctx, map, sizeof(map));
}
