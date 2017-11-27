
#include <zephyr/types.h>
#include <ztest.h>
#include <firmata.h>

extern int midi_to_u16(u8_t *buf, u16_t len, u16_t offset, u16_t *val);
extern int u16_to_midi(u8_t *buf, u16_t len, u16_t offset, u16_t val);

extern void test_output_cap_response(void);
extern void test_output_string_data(void);
extern void test_output_analog_mapping_response(void);

struct io_test_data {
	u8_t data[3];
	u8_t cmd;
	u8_t pin;
	u16_t val;
};

struct io_test_data aio_tests[] = {
	{
		.data = {0xE0, 0x00, 0x1},
		.cmd = 0xE0,
		.pin = 0x0,
		.val = 0x80,
	},
	{
		.data = {0xE1, 0x52, 0x0},
		.cmd = 0xE1,
		.pin = 0x1,
		.val = 0x52,
	},
	{
		.data = {0xE3, 0x63, 0x1},
		.cmd = 0xE3,
		.pin = 0x3,
		.val = 0xE3,
	},
	{
		.data = {0xEF, 0x7F, 0x1},
		.cmd = 0xEF,
		.pin = 0xF,
		.val = 0xFF,
	},
	{
		.data = {0xEF, 0x7F, 0x7F},
		.cmd = 0xEF,
		.pin = 0xF,
		.val = 0x3FFF,
	},
	{
		.data = {0xE2, 0x7F, 0x03},
		.cmd = 0xE2,
		.pin = 0x2,
		.val = 0x01FF,
	},
};

struct io_test_data dio_tests[] = {
	{
		.data = {0x90, 0x00, 0x1},
		.cmd = 0x90,
		.pin = 0x0,
		.val = 0x80,
	},
	{
		.data = {0x91, 0x52, 0x0},
		.cmd = 0x91,
		.pin = 0x1,
		.val = 0x52,
	},
	{
		.data = {0x93, 0x63, 0x1},
		.cmd = 0x93,
		.pin = 0x3,
		.val = 0xE3,
	},
	{
		.data = {0x9F, 0x7F, 0x1},
		.cmd = 0x9F,
		.pin = 0xF,
		.val = 0xFF,
	},
};

struct report_io_test_data {
	u8_t data[2];
	u8_t cmd;
	u8_t pin;
	u8_t val;
};

struct report_io_test_data report_aio_tests[] = {
	{
		.data = {0xC0, 0x00},
		.cmd = 0xC0,
		.pin = 0x0,
		.val = 0x00,
	},
	{
		.data = {0xC1, 0x01},
		.cmd = 0xC1,
		.pin = 0x1,
		.val = 0x01,
	},
	{
		.data = {0xC3, 0x01},
		.cmd = 0xC3,
		.pin = 0x3,
		.val = 0x01,
	},
	{
		.data = {0xCF, 0x01},
		.cmd = 0xCF,
		.pin = 0xF,
		.val = 0x01,
	},
};

struct report_io_test_data report_dio_tests[] = {
	{
		.data = {0xD0, 0x00},
		.cmd = 0xD0,
		.pin = 0x0,
		.val = 0x00,
	},
	{
		.data = {0xD1, 0x01},
		.cmd = 0xD1,
		.pin = 0x1,
		.val = 0x01,
	},
	{
		.data = {0xD3, 0x01},
		.cmd = 0xD3,
		.pin = 0x3,
		.val = 0x01,
	},
	{
		.data = {0xDF, 0x01},
		.cmd = 0xDF,
		.pin = 0xF,
		.val = 0x01,
	},
};

#define INPUT_IO_TEST_DATA(x) \
	do {                                                                \
		for (int i = 0; i < ARRAY_SIZE(x); i++) {                   \
			ctx.cb_user_data = &i;                              \
			ret = firmata_input_buf(&ctx,                       \
						x[i].data,                  \
						sizeof(x[i].data));         \
			zassert_equal(ret, 0, "firmata_input_buf() error"); \
		}                                                           \
	} while (0)

#define CB_CHECK_IO_TEST_DATA(x, c, p, v, u) \
	do {								\
		int *i = (int *)u;					\
									\
		zassert_not_null(i, "null user data");			\
		zassert_true(*i < ARRAY_SIZE(x), "index error");	\
		zassert_equal(c, x[*i].cmd, "cmd error");		\
		zassert_equal(p, x[*i].pin, "pin error");		\
		zassert_equal(v, x[*i].val, "val error");		\
	} while (0)

void test_midi_decode(void)
{
	/* Decode analog io message */
	for (int i = 0; i < ARRAY_SIZE(aio_tests); i++) {
		int ret;
		u16_t val;
		struct io_test_data *t = aio_tests + i;

		/* Zero-size buf test */
		ret = midi_to_u16(t->data, 0, 1, &val);
		zassert_equal(ret, -ENOBUFS, "midi_to_u16() error");

		/* Bounary test */
		ret = midi_to_u16(t->data, 1, 1, &val);
		zassert_equal(ret, -ENOBUFS, "midi_to_u16() error");

		ret = midi_to_u16(t->data, sizeof(t->data), 1, &val);
		zassert_equal(ret, 0, "midi_to_u16() error");

		zassert_equal(t->val, val, "decode error");
	}

	/* Decode digital io message */
	for (int i = 0; i < ARRAY_SIZE(dio_tests); i++) {
		int ret;
		u16_t val;
		struct io_test_data *t = dio_tests + i;

		/* Zero-size buf test */
		ret = midi_to_u16(t->data, 0, 1, &val);
		zassert_equal(ret, -ENOBUFS, "midi_to_u16() error");

		/* Bounary test */
		ret = midi_to_u16(t->data, 1, 1, &val);
		zassert_equal(ret, -ENOBUFS, "midi_to_u16() error");

		ret = midi_to_u16(t->data, sizeof(t->data), 1, &val);
		zassert_equal(ret, 0, "midi_to_u16() error");

		zassert_equal(t->val, val, "decode error");
	}

	/* TODO: out of bound test */
}

void test_midi_encode(void)
{
	/* Encode analog io message */
	for (int i = 0; i < ARRAY_SIZE(aio_tests); i++) {
		int ret;
		u8_t buf[3];
		struct io_test_data *t = aio_tests + i;

		buf[0] = t->cmd;

		/* Zero-size buf test */
		ret = u16_to_midi(buf, 1, 1, t->val);
		zassert_equal(ret, -ENOBUFS, "u16_to_midi() error");

		/* Bounary test */
		ret = u16_to_midi(buf, 1, 1, t->val);
		zassert_equal(ret, -ENOBUFS, "u16_to_midi() error");

		ret = u16_to_midi(buf, sizeof(buf), 1, t->val);
		zassert_equal(ret, 0, "u16_to_midi() error");

		zassert_equal(0, memcmp(t->data, buf, sizeof(buf)),
			      "encode error");
	}

	/* Encode digital io message */
	for (int i = 0; i < ARRAY_SIZE(dio_tests); i++) {
		int ret;
		u8_t buf[3];
		struct io_test_data *t = dio_tests + i;

		buf[0] = t->cmd;

		/* Zero-size buf test */
		ret = u16_to_midi(buf, 1, 1, t->val);
		zassert_equal(ret, -ENOBUFS, "u16_to_midi() error");

		/* Bounary test */
		ret = u16_to_midi(buf, 1, 1, t->val);
		zassert_equal(ret, -ENOBUFS, "u16_to_midi() error");

		ret = u16_to_midi(buf, sizeof(buf), 1, t->val);
		zassert_equal(ret, 0, "u16_to_midi() error");

		zassert_equal(0, memcmp(t->data, buf, sizeof(buf)),
			      "encode error");
	}

	/* TODO: out of bound test */
}

int test_aio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin, u16_t val,
		void *user_data)
{
	CB_CHECK_IO_TEST_DATA(aio_tests, cmd, pin, val, user_data);

	return 0;
}

void test_aio(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs = { 0 };
	int ret;

	cbs.aio = test_aio_cb;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.aio = test_aio_cb;
	firmata_attach_callback(&ctx, &cbs);

	INPUT_IO_TEST_DATA(aio_tests);
}

int test_dio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin, u16_t val,
		void *user_data)
{
	CB_CHECK_IO_TEST_DATA(dio_tests, cmd, pin, val, user_data);

	return 0;
}

void test_dio(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs = { 0 };
	int ret;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.dio = test_dio_cb;
	firmata_attach_callback(&ctx, &cbs);

	INPUT_IO_TEST_DATA(dio_tests);
}

int test_report_aio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
		       u16_t val, void *user_data)
{
	CB_CHECK_IO_TEST_DATA(report_aio_tests, cmd, pin, val, user_data);

	return 0;
}

void test_report_aio(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs = { 0 };
	int ret;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.report_aio = test_report_aio_cb;
	firmata_attach_callback(&ctx, &cbs);

	INPUT_IO_TEST_DATA(report_aio_tests);
}

int test_report_dio_cb(struct firmata_context *ctx, u8_t cmd, u8_t pin,
		       u16_t val, void *user_data)
{
	CB_CHECK_IO_TEST_DATA(report_dio_tests, cmd, pin, val, user_data);

	return 0;
}

void test_report_dio(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs = { 0 };
	int ret;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.report_dio = test_report_dio_cb;
	firmata_attach_callback(&ctx, &cbs);

	INPUT_IO_TEST_DATA(report_dio_tests);
}

void test_main(void)
{
	/* TODO: test_output_dio_msg and test_output_aio_msg */

	ztest_test_suite(firmata_test,
			 ztest_unit_test(test_midi_decode),
			 ztest_unit_test(test_midi_encode),
			 ztest_unit_test(test_aio),
			 ztest_unit_test(test_dio),
			 ztest_unit_test(test_report_aio),
			 ztest_unit_test(test_report_dio),
			 ztest_unit_test(test_output_string_data),
			 ztest_unit_test(test_output_cap_response),
			 ztest_unit_test(test_output_analog_mapping_response));

	ztest_run_test_suite(firmata_test);
}
