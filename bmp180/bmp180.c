#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>

s16 oss = 0; // oversampling setting (0 - 3 )

struct data {
	struct i2c_client *drv_client;
	u32 pressure;
	u32 temperature;
} bmp180_data;


struct calib {
	s16 AC1;
	s16 AC2;
	s16 AC3;
	u16 AC4;
	u16 AC5;
	u16 AC6;
	s16 B1;
	s16 B2;
	s16 MB;
	s16 MC;
	s16 MD;
} bmp180_calib;
static int bmp180_probe(struct i2c_client *drv_client, const struct i2c_device_id *id)
{
	int ret;
	printk(KERN_DEBUG "PROBE");
	dev_info(&drv_client->dev,
		"i2c client address is 0x%X\n", drv_client->addr);
    
	/* Read callibration coefficients */
	u16 res_msb = i2c_smbus_read_byte_data(drv_client, 0xAA);
    u16 res_lsb = i2c_smbus_read_byte_data(drv_client, 0xAB);
    bmp180_calib.AC1 = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "AC1 : %d\n", bmp180_calib.AC1);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xAC);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xAD);
    bmp180_calib.AC2 = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "AC2 : %d\n", bmp180_calib.AC2);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xAE);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xAF);
    bmp180_calib.AC3 = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "AC3 : %d\n", bmp180_calib.AC3);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xB0);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xB1);
    bmp180_calib.AC4 = ((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "AC4 : %d\n", bmp180_calib.AC4);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xB2);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xB3);
    bmp180_calib.AC5 = ((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "AC5 : %d\n", bmp180_calib.AC5);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xB4);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xB5);
    bmp180_calib.AC6 = ((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "AC6 : %d\n", bmp180_calib.AC6);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xB6);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xB7);
    bmp180_calib.B1 = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "B1 : %d\n", bmp180_calib.B1);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xB8);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xB9);
    bmp180_calib.B2 = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "B2 : %d\n", bmp180_calib.B2);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xBA);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xBB);
    bmp180_calib.MB = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "MB : %d\n", bmp180_calib.MB);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xBC);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xBD);
    bmp180_calib.MC = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "MC : %d\n", bmp180_calib.MC);

    res_msb = i2c_smbus_read_byte_data(drv_client, 0xBE);
    res_lsb = i2c_smbus_read_byte_data(drv_client, 0xBF);
    bmp180_calib.MD = (s16)((res_msb << 8) + res_lsb);
    dev_info(&drv_client->dev, "MD : %d\n", bmp180_calib.MD);

	
	bmp180_data.drv_client = drv_client;

	dev_info(&drv_client->dev, "i2c driver probed\n");
	return 0;
}


static int bmp180_read_temp_and_press(void)
{
	struct i2c_client *drv_client = bmp180_data.drv_client;
	if (drv_client == 0)
		return -ENODEV;
	printk(KERN_INFO "Started reading data\n");
	//Reading UT(Uncompensated temerature)
	i2c_smbus_write_byte_data(drv_client, 0xF4, 0x2E);
	mdelay(5);
	u16 MSB = i2c_smbus_read_byte_data(drv_client, 0xF6);
	u16 LSB = i2c_smbus_read_byte_data(drv_client, 0xF7);
	s32 UT = (MSB << 8) + LSB;
	printk(KERN_INFO "UT : %d\n", UT);

	//Choosing delay for current OSS
	int delay_for_press;
	if (oss == 0)
		delay_for_press = 5;
	else if (oss == 1)
		delay_for_press = 8;
	else if (oss == 2)
		delay_for_press = 14;
	else 
		delay_for_press = 26;
	//Reading UP(Uncompensated pressure)
	i2c_smbus_write_byte_data(drv_client, 0xF4, 0x34 + (oss << 6));
	mdelay(delay_for_press);
	MSB = i2c_smbus_read_byte_data(drv_client, 0xF6);
	LSB = i2c_smbus_read_byte_data(drv_client, 0xF7);
	u16 XLSB = i2c_smbus_read_byte_data(drv_client, 0xF8);
	s32 UP = (((MSB << 16) + (LSB << 8)) + XLSB) >> (8 - oss);

	//Calculate true temperature
	s32 X1 = ((UT - bmp180_calib.AC6) * bmp180_calib.AC5) >> 15;
	s32 X2 = (bmp180_calib.MC << 11) / (X1 + bmp180_calib.MD);
	s32 B5 = X1 + X2;
	s32 TRUE_TEMP = (B5 + 8) >> 4;

	//Calculate true pressure
	s32 B6 = B5 - 4000;
	X1 = (bmp180_calib.B2 * (B6 * B6 >> 12)) >> 11;
	X2 = bmp180_calib.AC2 * B6 >> 11;
	s32 X3 = X1 + X2;
	s32 B3 = ((((s32)bmp180_calib.AC1 * 4 + X3) << oss) + 2) / 4;
	X1 = bmp180_calib.AC3 * B6 >> 13;
	X2 = (bmp180_calib.B1 * ((B6 * B6) >> 12)) >> 16; // FSDFSDF
	X3 = (X1 + X2 + 2) >> 2;
	u32 B4 = bmp180_calib.AC4 * (u32)(X3 + 32768) >> 15;
	u32 B7 = ((u32)UP - B3) * (50000 >> oss);

	s32 TRUE_PRESSURE;
	if (B7 < 0x8000000) {
		TRUE_PRESSURE = (B7 * 2) / B4;
	}
	else {
		TRUE_PRESSURE = (B7 / B4) * 2;
	}
	X1 = (TRUE_PRESSURE >> 8) * (TRUE_PRESSURE >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * TRUE_PRESSURE) >> 16;
	TRUE_PRESSURE = TRUE_PRESSURE + ((X1 + X2 + 3791) >> 4);

	bmp180_data.temperature = TRUE_TEMP;
	bmp180_data.pressure = TRUE_PRESSURE;
	return 0;
}


//static struct bmp180_data g_mpu6050_data;


static ssize_t temp_show(struct class *class,
			    struct class_attribute *attr, char *buf)
{
	bmp180_read_temp_and_press();
	sprintf(buf, "%d.%d\n", bmp180_data.temperature / 10,
							 bmp180_data.temperature % 10);
	return strlen(buf);
}

static ssize_t press_in_pa_show(struct class *class,
			    struct class_attribute *attr, char *buf)
{
	bmp180_read_temp_and_press();
	sprintf(buf, "%d\n", bmp180_data.pressure);
	return strlen(buf);
}

static ssize_t press_in_mm_rt_st_show(struct class *class,
			    struct class_attribute *attr, char *buf)
{
	bmp180_read_temp_and_press(); // MM_RT_ST = PA / 133.3224
	sprintf(buf, "%d\n", (bmp180_data.pressure * 10000) / 1333224);
	return strlen(buf);
}


static ssize_t oversampling_show(struct class *class,
			    struct class_attribute *attr, char *buf)
{
	sprintf(buf, "%d\n", oss);
	return strlen(buf);
}

static ssize_t oversampling_store(struct class *class,
				struct class_attribute *attr, const char *buf , size_t size)
{
	long res;
	char *temp = kmalloc(2, GFP_KERNEL);
	temp[0] = buf[0];
	temp[1] = '\0';
	kstrtol(temp, 10, &res);
	if (res < 0 || res > 3) {
		printk(KERN_ERR "Oversampling rate can be only from 0 to 3\n");
		return size;
	}
	kfree(temp);
	oss = res;
	return size;
}



static int bmp180_remove(struct i2c_client *drv_client)
{
	bmp180_data.drv_client = 0;

	dev_info(&drv_client->dev, "i2c driver removed\n");
	return 0;
}

static const struct i2c_device_id bmp180_idtable[] = {
	{ "bmp180", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, bmp180_idtable);

static struct i2c_driver bmp180_i2c_driver = {
	.driver = {
		.name = "bmp180_i2c",
	},

	.probe = bmp180_probe,
	.remove = bmp180_remove,
	.id_table = bmp180_idtable,
};



static struct class_attribute class_attr_temp = __ATTR(temp, S_IRUGO, temp_show, NULL);
static struct class_attribute class_attr_press_in_pa = __ATTR(press_in_pa, S_IRUGO, press_in_pa_show, NULL);
static struct class_attribute class_attr_oversapmpling = __ATTR(oversampling, (S_IRUGO | S_IWUSR),
																 oversampling_show, oversampling_store);

static struct class_attribute class_attr_press_in_mm_rt_st = __ATTR(press_in_mm_rt_st, S_IRUGO, press_in_mm_rt_st_show, NULL);



//Pointer to directory in /sys/class
static struct class *class_dir;

static int bmp180_init(void)
{
	int ret;

	/* Create i2c driver */
	ret = i2c_add_driver(&bmp180_i2c_driver);
	if (ret) {
		pr_err("mpu6050: failed to add new i2c driver: %d\n", ret);
		return ret;
	}
	pr_info("mpu6050: i2c driver created\n");

	/* Create class */
	class_dir = class_create(THIS_MODULE, "bmp180");
	if (IS_ERR(class_dir)) {
		ret = PTR_ERR(class_dir);
		pr_err("bmp180: failed to create sysfs class: %d\n", ret);
		return ret;
	}
	pr_info("bmp180: sysfs class created\n");

	/* Create temp */
	ret = class_create_file(class_dir, &class_attr_temp);
	if (ret) {
		pr_err("bmp180: failed to create sysfs class attribute temp: %d\n", ret);
		return ret;
	}
	/* Create press */
	ret = class_create_file(class_dir, &class_attr_press_in_pa);
	if (ret) {
		pr_err("bmp180: failed to create sysfs class attribute press_in_pa: %d\n", ret);
		return ret;
	}
	/* Create press_in_mm_rt_st*/
	ret = class_create_file(class_dir, &class_attr_press_in_mm_rt_st);
	if (ret) {
		pr_err("bmp180: failed to create sysfs class attribute press_in_mm_rt_st: %d\n", ret);
		return ret;
	}
	/* Create oversampling*/
	ret = class_create_file(class_dir, &class_attr_oversapmpling);
	if (ret) {
		pr_err("bmp180: failed to create sysfs class attribute oversampling: %d\n", ret);
		return ret;
	}
	
	pr_info("bmp180: sysfs class attributes created\n");
	pr_info("bmp180: module loaded\n");
	return 0;
}

static void bmp180_exit(void)
{
	if (class_dir) {
		
		class_remove_file(class_dir, &class_attr_temp);
		class_remove_file(class_dir, &class_attr_press_in_pa);
		class_remove_file(class_dir, &class_attr_oversapmpling);
		class_remove_file(class_dir, &class_attr_press_in_mm_rt_st);
		pr_info("bmp180: sysfs class attributes removed\n");

		class_destroy(class_dir);
		pr_info("bm180: sysfs class destroyed\n");
	}
	i2c_del_driver(&bmp180_i2c_driver);
	pr_info("bmp180: i2c driver deleted\n");

	pr_info("bmp180: module exited\n");
}

module_init(bmp180_init);
module_exit(bmp180_exit);

MODULE_AUTHOR("Eduard.Voronkin <eduard.voronkin@nure.ua>");
MODULE_DESCRIPTION("bmp180 I2C");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

