// SPDX-License-Identifier: GPL-2.0-only
/*
 * Minimal Hynitron CST9217 Touchscreen Driver
 * This example is from the open-source sharing by engineers of Yuying Optoelectronics (Github.com/osptek).
 * Suggestions for improvement are welcome.
 * Supports single-touch (primary) and basic multi-touch reporting.
 * I2C address default: 0x5A
 *
 * 最小化 Hynitron CST9217 触摸屏驱动
 * 本例程来源于鱼鹰光电的工程师的开源分享 Github.com/osptek，欢迎提出改进意见
 * 支持单点触摸（主要）和基本多点触摸上报
 * 默认 I2C 地址：0x5A
 *
 * TP_RST / TP_INT are optional.
 * By default they are not used. Enable them only when the pins are actually connected.
 * TP_RST / TP_INT 均为可选。
 * 默认不使用，只有实际接了对应引脚时才启用。
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/input/touchscreen.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/unaligned.h>
#include <linux/workqueue.h>		/* for polling when no IRQ */

#define CST9217_DATA_REG		0xD000
#define CST9217_ACK_VALUE		0xAB
#define CST9217_MAX_POINTS		5
#define CST9217_DATA_LEN		(CST9217_MAX_POINTS * 5 + 5)
#define CST9217_POLL_INTERVAL_MS	20	/* 50 Hz when no interrupt */

struct cst9217_data {
	struct i2c_client		*client;
	struct input_dev		*input;
	struct gpio_desc		*reset_gpio;	/* optional */
	struct touchscreen_properties	prop;
	struct mutex			mutex;
	struct delayed_work		poll_work;	/* used only when no IRQ */
	bool				use_irq;
};

static int cst9217_i2c_read_reg(struct i2c_client *client, u16 reg,
				u8 *buf, u16 len)
{
	u8 reg_buf[2] = { reg >> 8, reg & 0xff };
	struct i2c_msg msgs[2] = {
		{
			.addr  = client->addr,
			.flags = 0,
			.len   = 2,
			.buf   = reg_buf,
		},
		{
			.addr  = client->addr,
			.flags = I2C_M_RD,
			.len   = len,
			.buf   = buf,
		},
	};
	int ret, retries = 3;

	while (retries--) {
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret == 2)
			return 0;
		usleep_range(2000, 3000);
	}
	return ret < 0 ? ret : -EIO;
}

static int cst9217_i2c_write_reg(struct i2c_client *client, u16 reg,
				 const u8 *data, u16 len)
{
	u8 buf[4];
	int ret, retries = 3;

	if (len > 2)
		return -EINVAL;

	buf[0] = reg >> 8;
	buf[1] = reg & 0xff;
	if (len)
		memcpy(&buf[2], data, len);

	while (retries--) {
		ret = i2c_master_send(client, buf, 2 + len);
		if (ret == 2 + len)
			return 0;
		usleep_range(2000, 3000);
	}
	return ret < 0 ? ret : -EIO;
}

/* Optional hardware reset. Do nothing if reset-gpios is not present. */
static void cst9217_reset(struct cst9217_data *ts)
{
	if (!ts->reset_gpio)
		return;

	/* Active-low reset sequence (common). Change levels if your board is active-high. */
	gpiod_set_value_cansleep(ts->reset_gpio, 0);
	msleep(10);
	gpiod_set_value_cansleep(ts->reset_gpio, 1);
	msleep(50);
}

static int cst9217_read_touch_data(struct cst9217_data *ts)
{
	u8 data[CST9217_DATA_LEN] = {0};
	struct input_dev *input = ts->input;
	int ret, i, points;

	ret = cst9217_i2c_read_reg(ts->client, CST9217_DATA_REG,
				   data, sizeof(data));
	if (ret)
		return ret;

	if (data[6] != CST9217_ACK_VALUE)
		return 0;	/* no valid frame, just ignore */

	points = data[5] & 0x7f;
	if (points > CST9217_MAX_POINTS)
		points = CST9217_MAX_POINTS;

	for (i = 0; i < points; i++) {
		u8 *p = &data[i * 5 + (i ? 2 : 0)];
		u8 status = p[0] & 0x0f;
		u16 x, y;

		if (status != 0x06)
			continue;

		x = (p[1] << 4) | (p[3] >> 4);
		y = (p[2] << 4) | (p[3] & 0x0f);

		input_mt_slot(input, i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
		touchscreen_report_pos(input, &ts->prop, x, y, true);
	}

	input_mt_sync_frame(input);
	input_sync(input);
	return 0;
}

static irqreturn_t cst9217_irq_handler(int irq, void *dev_id)
{
	struct cst9217_data *ts = dev_id;

	mutex_lock(&ts->mutex);
	cst9217_read_touch_data(ts);
	mutex_unlock(&ts->mutex);

	return IRQ_HANDLED;
}

/* Polling work when no interrupt is available */
static void cst9217_poll_work(struct work_struct *work)
{
	struct cst9217_data *ts = container_of(to_delayed_work(work),
					       struct cst9217_data, poll_work);

	mutex_lock(&ts->mutex);
	cst9217_read_touch_data(ts);
	mutex_unlock(&ts->mutex);

	schedule_delayed_work(&ts->poll_work,
			      msecs_to_jiffies(CST9217_POLL_INTERVAL_MS));
}

static int cst9217_probe(struct i2c_client *client)
{
	struct cst9217_data *ts;
	struct input_dev *input;
	int ret;

	ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	ts->client = client;
	mutex_init(&ts->mutex);
	i2c_set_clientdata(client, ts);

	/* Optional reset GPIO – not required */
	ts->reset_gpio = devm_gpiod_get_optional(&client->dev, "reset",
						 GPIOD_OUT_HIGH);
	if (IS_ERR(ts->reset_gpio))
		return PTR_ERR(ts->reset_gpio);

	cst9217_reset(ts);	/* does nothing if reset_gpio is NULL */

	/* Optional chip info read (for debug only) */
	{
		u8 cmd[2] = {0xD1, 0x01};
		u8 buf[4];
		cst9217_i2c_write_reg(client, 0xD101, cmd, 2);
		msleep(10);
		if (!cst9217_i2c_read_reg(client, 0xD1FC, buf, 4))
			dev_info(&client->dev, "checkcode %02x%02x%02x%02x\n",
				 buf[0], buf[1], buf[2], buf[3]);
	}

	input = devm_input_allocate_device(&client->dev);
	if (!input)
		return -ENOMEM;

	ts->input = input;
	input->name = "Hynitron CST9217 Touchscreen";
	input->id.bustype = BUS_I2C;
	input->dev.parent = &client->dev;

	input_set_capability(input, EV_KEY, BTN_TOUCH);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, 466, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, 466, 0, 0);

	touchscreen_parse_properties(input, true, &ts->prop);

	ret = input_mt_init_slots(input, CST9217_MAX_POINTS,
				  INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);
	if (ret)
		return ret;

	ret = input_register_device(input);
	if (ret)
		return ret;

	/* Interrupt is optional. Fall back to polling when not present. */
	if (client->irq > 0) {
		ret = devm_request_threaded_irq(&client->dev, client->irq,
						NULL, cst9217_irq_handler,
						IRQF_ONESHOT | IRQF_TRIGGER_FALLING,
						"cst9217", ts);
		if (ret) {
			dev_warn(&client->dev,
				 "failed to request IRQ %d, fall back to polling\n",
				 client->irq);
			client->irq = 0;
		} else {
			ts->use_irq = true;
			dev_info(&client->dev, "using interrupt mode (IRQ %d)\n",
				 client->irq);
		}
	}

	if (!ts->use_irq) {
		INIT_DELAYED_WORK(&ts->poll_work, cst9217_poll_work);
		schedule_delayed_work(&ts->poll_work,
				      msecs_to_jiffies(CST9217_POLL_INTERVAL_MS));
		dev_info(&client->dev, "using polling mode (%d ms)\n",
			 CST9217_POLL_INTERVAL_MS);
	}

	dev_info(&client->dev, "CST9217 touchscreen registered (addr 0x%02x)\n",
		 client->addr);
	return 0;
}

static void cst9217_remove(struct i2c_client *client)
{
	struct cst9217_data *ts = i2c_get_clientdata(client);

	if (!ts->use_irq)
		cancel_delayed_work_sync(&ts->poll_work);
}

static const struct of_device_id cst9217_of_match[] = {
	{ .compatible = "hynitron,cst9217" },
	{ }
};
MODULE_DEVICE_TABLE(of, cst9217_of_match);

static const struct i2c_device_id cst9217_id[] = {
	{ "cst9217", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, cst9217_id);

static struct i2c_driver cst9217_driver = {
	.driver = {
		.name           = "hynitron_cst9217",
		.of_match_table = cst9217_of_match,
	},
	.probe    = cst9217_probe,
	.remove   = cst9217_remove,
	.id_table = cst9217_id,
};
module_i2c_driver(cst9217_driver);

MODULE_AUTHOR("Adapted from open-source sharing by Yuying Optoelectronics (Github.com/osptek)");
MODULE_DESCRIPTION("Hynitron CST9217 touchscreen driver");
MODULE_LICENSE("GPL");