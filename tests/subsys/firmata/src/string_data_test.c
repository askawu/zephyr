
#include <zephyr/types.h>
#include <ztest.h>
#include <firmata.h>

struct string_test_data {
	u8_t data[100];
	u16_t data_size;
	char *string;
};

static struct string_test_data string_data_tests[] = {
	{
		.data = { 0xF0, 0x71, 0x41, 0x00, 0xF7 },
		.data_size = 5,
		.string = "A",
	},
	{
		.data = { 0xF0, 0x71, 0x41, 0x00, 0x42,
			  0x00, 0x43, 0x00, 0xF7 },
		.data_size = 9,
		.string = "ABC",
	},
	{
		.data = { 0xF0, 0x71,
			  0x54, 0x00, 0x68, 0x00, 0x69, 0x00, 0x73, 0x00,
			  0x20, 0x00, 0x69, 0x00, 0x73, 0x00, 0x20, 0x00,
			  0x61, 0x00, 0x20, 0x00, 0x66, 0x00, 0x69, 0x00,
			  0x72, 0x00, 0x6d, 0x00, 0x61, 0x00, 0x74, 0x00,
			  0x61, 0x00, 0x20, 0x00, 0x73, 0x00, 0x74, 0x00,
			  0x72, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x67, 0x00,
			  0x20, 0x00, 0x64, 0x00, 0x61, 0x00, 0x74, 0x00,
			  0x61, 0x00, 0x20, 0x00, 0x74, 0x00, 0x65, 0x00,
			  0x73, 0x00, 0x74, 0x00,
			  0xF7 },
		.data_size = 71,
		.string = "This is a firmata string data test",
	},
};

/* Current test item in string_data_tests */
static int curr_test_index = 0;

/* Current data to be compared with output data */
static int curr_data_index = 0;

static int test_output_cb(struct firmata_context *ctx, const u8_t *buf,
			  u16_t len, void *user_data)
{
	int *i = (int *)user_data;
	int ret;

	/* The output callback might be called with paritial msg data.
	 * curr_test_index and curr_data_index are used to keep track of
	 * next expected data.
	 */

	zassert_not_null(i, "null user data");
	zassert_true(*i < ARRAY_SIZE(string_data_tests), "index error");

	if (curr_test_index != *i) {
		/* Reset curr data index if it's new test item */
		curr_test_index = *i;
		curr_data_index = 0;
	}

	ret = memcmp(buf, string_data_tests[*i].data + curr_data_index, len);
	zassert_equal(ret, 0, "data error");

	/* Advance data index */
	curr_data_index += len;

	return 0;
}

void test_output_string_data(void)
{
	struct firmata_context ctx;
	struct firmata_callback cbs;
	int ret;

	ret = firmata_init(&ctx);
	zassert_equal(ret, 0, "firmata_init() error");

	cbs.output = test_output_cb;
	firmata_attach_callback(&ctx, &cbs);

	for (int i = 0; i < ARRAY_SIZE(string_data_tests); i++) {
		ctx.cb_user_data = &i;
		firmata_output_string_data(&ctx, string_data_tests[i].string);
	}
}
