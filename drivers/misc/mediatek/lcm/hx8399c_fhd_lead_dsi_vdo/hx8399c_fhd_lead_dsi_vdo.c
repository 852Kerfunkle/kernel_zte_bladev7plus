#ifdef BUILD_LK
#include <string.h>
#include <mt_gpio.h>
#include <platform/mt_pmic.h>
#include <cust_gpio_usage.h>
#include <platform/gpio_const.h>
#include <platform/mt_i2c.h>
#include <platform/upmu_common.h>
#include <cust_i2c.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <linux/string.h>
#include <mt-plat/mt_gpio.h>
#include <mach/gpio_const.h>
//#include <mach/mt_gpio.h>
//#include <mach/mt_pm_ldo.h>
#endif

#include "lcm_drv.h"
//#include <cust_gpio_usage.h>
#if defined(BUILD_LK)
#define LCM_PRINT printf
#elif defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH (1080) // pixel
#define FRAME_HEIGHT (1920) // pixel

#define REGFLAG_DELAY 0xFFAB
#define REGFLAG_END_OF_TABLE 0xFFAA // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                  lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)              lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg                                          lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define GPIO_LCD_ENP         (GPIO2 | 0x80000000)
#define GPIO_LCD_ENN         (GPIO57 | 0x80000000)
#define GPIO_LCDBL_EN_PIN         (GPIO86 | 0x80000000)

//#define LCM_DSI_CMD_MODE
#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/jiffies.h>
#include <linux/uaccess.h>
//#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
/*****************************************************************************
 * Define
 *****************************************************************************/
#ifdef CONFIG_MTK_LEGACY
#define LCM_I2C_ADDR 0x3E
#define LCM_I2C_BUSNUM  3	/* for I2C channel 0 */
#define LCM_I2C_ID_NAME "tps65132"
#else
#define LCM_I2C_ADDR 0x3E
#define LCM_I2C_BUSNUM  3	/* for I2C channel 3 */
#define LCM_I2C_ID_NAME "I2C_LCD_BIAS"
#endif


/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
#ifdef CONFIG_MTK_LEGACY
static struct i2c_board_info _lcm_i2c_board_info __initdata = {
	I2C_BOARD_INFO(LCM_I2C_ID_NAME, LCM_I2C_ADDR)
};
#else
static const struct of_device_id _lcm_i2c_of_match[] = {
	{
	 .compatible = "mediatek,I2C_LCD_BIAS",
	 },
};
#endif

static struct i2c_client *_lcm_i2c_client;


/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int _lcm_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int _lcm_i2c_remove(struct i2c_client *client);


/*****************************************************************************
 * Data Structure
 *****************************************************************************/
struct _lcm_i2c_dev {
	struct i2c_client *client;

};


static const struct i2c_device_id _lcm_i2c_id[] = {
	{LCM_I2C_ID_NAME, 0},
	{}
};


/* #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)) */
/* static struct i2c_client_address_data addr_data = { .forces = forces,}; */
/* #endif */
static struct i2c_driver _lcm_i2c_driver = {
	.id_table = _lcm_i2c_id,
	.probe = _lcm_i2c_probe,
	.remove = _lcm_i2c_remove,
	/* .detect               = _lcm_i2c_detect, */
	.driver = {
		   .owner = THIS_MODULE,
		   .name = LCM_I2C_ID_NAME,
#ifdef CONFIG_MTK_LEGACY
#else
		   .of_match_table = _lcm_i2c_of_match,
#endif
		   },

};


/*****************************************************************************
 * Extern Area
 *****************************************************************************/



/*****************************************************************************
 * Function
 *****************************************************************************/
static int _lcm_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	LCM_PRINT("[LCM][I2C] _lcm_i2c_probe\n");
	//pr_debug("[LCM][I2C] NT: info==>name=%s addr=0x%x\n", client->name, client->addr);
	_lcm_i2c_client = client;
	return 0;
}


static int _lcm_i2c_remove(struct i2c_client *client)
{
	LCM_PRINT("[LCM][I2C] _lcm_i2c_remove\n");
	_lcm_i2c_client = NULL;
	i2c_unregister_device(client);
	return 0;
}


static int _lcm_i2c_write_bytes(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = _lcm_i2c_client;
	char write_data[2] = { 0 };

	write_data[0] = addr;
	write_data[1] = value;
	ret = i2c_master_send(client, write_data, 2);
	if (ret < 0)
		LCM_PRINT("[LCM][ERROR] _lcm_i2c write data fail !!\n");

	return ret;
}


/*
 * module load/unload record keeping
 */
static int __init _lcm_i2c_init(void)
{
	LCM_PRINT("[LCM][I2C] _lcm_i2c_init\n");
#ifdef CONFIG_MTK_LEGACY
	i2c_register_board_info(LCM_I2C_BUSNUM, &_lcm_i2c_board_info, 1);
	LCM_PRINT("[LCM][I2C] _lcm_i2c_init2\n");
#endif
	if(i2c_add_driver(&_lcm_i2c_driver))
{
		LCM_PRINT("add driver error\n");
		return -1;
	}
	LCM_PRINT("[LCM][I2C] _lcm_i2c_init success\n");

	return 0;
}


static void __exit _lcm_i2c_exit(void)
{
	LCM_PRINT("[LCM][I2C] _lcm_i2c_exit\n");
	i2c_del_driver(&_lcm_i2c_driver);
}

module_init(_lcm_i2c_init);
module_exit(_lcm_i2c_exit);

MODULE_AUTHOR("Joey Pan");
MODULE_DESCRIPTION("MTK LCM I2C Driver");
MODULE_LICENSE("GPL");

#endif

#ifdef BUILD_LK
#define TPS65132_SLAVE_ADDR_WRITE  0x7C
static struct mt_i2c_t TPS65132_i2c;

int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    TPS65132_i2c.id = I2C_I2C_LCD_BIAS_CHANNEL;//I2C2;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    TPS65132_i2c.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
    TPS65132_i2c.mode = ST_MODE;
    TPS65132_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&TPS65132_i2c, write_data, len);
    //printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}
#endif
struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

{0xB9,3,{0xFF, 0x83, 0x99}},

     {0xBA,4,{0x63,0x03,0x68,0x73}},	
//,0xSet,0xPower
     //Generic_Long_Write_FIFO(11,B1);
{0xB1,15,{0x02,0x04,0x6D,0x8D,0x01,0x32,0x33,0x11,0x11,0x5E,0x64,0x56,0x73,0x02,0x02}},

//,0xSet,0xMIPI	

   //{0xD2,1,{0x88}},

//,0xSet,0xDisplay
  {0xB2,15,{0x07,0x00,0x80,0xAE,0x05,0x07,0x5A,0x31,0x10,0x30,0x00,0x1E,0x70,0x03,0xD4}},


//,0xSet,0xCYC
 //    Generic_Long_Write_FIFO(22,B4);
{0xB4,44,{0x00,0xFF,0x10,0x18,0x04,0x9A,0x00,0x00,0x06,0x00,0x02,0x04,0x00,0x24,0x02,0x04,0x0A,0x21,0x03,0x00,0x00,0x02,0x9F,0x88,0x10,0x18,0x04,0x9A,0x00,0x00,0x08,0x00,0x02,0x04,0x00,0x24,0x02,0x04,0x0A,0x00,0x00,0x02,0x9F,0x12}},

//,0xSet,0xVDC
 //  Generic_Short_Write_1P(0xBC,0x07);

//,0xSet,0xPower,0xOption
    // Generic_Long_Write_3P(0xBF,0x41,0x0E,0x01);

//,0xSet,0xD3
 //   Generic_Long_Write_FIFO(34,D3);
{0xD3,33,{0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x05,0x00,0x05,0x00,0x07,0x88,0x07,0x88,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x05,0x40}},


//,0xSet,0xGIP
{0xD5,32,{0x20,0x20,0x1E,0x1E,0x1F,0x1F,0x01,0x01,0x00,0x00,0x25,0x25,0x18,0x18,0x18,0x18,0x24,0x24,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x2F,0x2F,0x30,0x30,0x31,0x31}},


//,0xSet,0xD6
 //  Generic_Long_Write_FIFO(45,D6);
{0xD6,32,{0x24,0x24,0x1E,0x1E,0x1F,0x1F,0x00,0x00,0x01,0x01,0x25,0x25,0x18,0x18,0x18,0x18,0x20,0x20,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x2F,0x2F,0x30,0x30,0x31,0x31}},

 {0xBD,1,{0x00}},
 
{0xD8,16,{0xAA,0x8A,0xAA,0xAA,0xAA,0x8A,0xAA,0xAA,0xAA,0x8A,0xAA,0xAA,0xAA,0x8A,0xAA,0xAA}},

 {0xBD,1,{0x01}},
 
{0xD8,16,{0xFF,0xCF,0xEA,0xBF,0xFF,0xCF,0xEA,0xBF,0xFF,0xCF,0xEA,0xBF,0xFF,0xCF,0xEA,0xBF}},

 {0xBD,1,{0x02}},
 
{0xD8,8,{0xFF,0xCF,0xEA,0xBF,0xFF,0xCF,0xEA,0xBF}},
//,0xSet,0xGamma
 {0xBD,1,{0x00}},
 {0xC9,3,{0x03,0x00,0x12}},
 
{0xE0,54,{0x02,0x21,0x2D,0x28,0x58,0x62,0x6F,0x6B,0x74,0x7E,
0x84,0x8C,0x91,0x9A,0xA3,0xA8,0xAB,0xB3,0xB4,0xBC,
0xAD,0xBD,0xBF,0x64,0x62,0x6D,0x7C,0x02,0x21,0x2D,
0x28,0x59,0x62,0x6F,0x6C,0x74,0x7D,0x86,0x8C,0x90,
0x98,0x9F,0xA2,0xA8,0xB1,0xB4,0xBA,0xAE,0xBD,0xBE,
0x61,0x50,0x69,0x7C}},

//,0xSet Fliceke
{0xB6,2,{0x8A,0x8A}},

//,0xSet,0xPanel
{0xCC,1,{0x08}},
//,0xSet,0xC0
{0xC7,5,{0x00,0x08,0x00,0x01,0x08}},
{0xC0,2,{0x25,0x5A}},


{0xE4,1,{0x01}},
{REGFLAG_DELAY, 5, {}},
//	{0X3A, 1, {0X77}},	//Display Access Control
	{0X51, 2, {0x00,0x00}},	//Write Display Brightness Value
{REGFLAG_DELAY, 5, {}},
	{0X53, 1, {0x24}},	//Write CTRL Display Value
{REGFLAG_DELAY, 5, {}},
	{0X55, 1, {0x01}},	//Write Content Adaptive Brightness Control Value
{REGFLAG_DELAY, 5, {}},

{0x11,	1,	{0x00}},	
{REGFLAG_DELAY, 120, {}},	
{0x29,	1,	{0x00}},	
{REGFLAG_DELAY, 20, {}},
{0xE4,2,{0x01,0x01}},	
{REGFLAG_DELAY, 5, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;
		
		cmd = table[i].cmd;

		switch (cmd) {
			case REGFLAG_DELAY :
				MDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE :
				break;

			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy((void*)&lcm_util, (void*)util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->physical_width  = 64;//PHYSICAL_WIDTH;
       params->physical_height = 115;//PHYSICAL_HIGHT;
	// enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;

	params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;
	//params->dsi.mode   					= BURST_VDO_MODE;
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM                = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.

	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	params->dsi.packet_size=256;
	// Video mode setting
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4; 
	params->dsi.vertical_backporch = 4; 
	params->dsi.vertical_frontporch = 9;
	params->dsi.vertical_active_line                = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 60;
	params->dsi.horizontal_frontporch				= 90;
	params->dsi.horizontal_active_pixel             = FRAME_WIDTH;

	// Bit rate calculation
	//params->dsi.pll_div1=35;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	//params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)

	/* ESD or noise interference recovery For video mode LCM only. */
	// Send TE packet to LCM in a period of n frames and check the response.
	//params->dsi.lcm_int_te_monitor = FALSE;
	//params->dsi.lcm_int_te_period = 1;		// Unit : frames

	// Need longer FP for more opportunity to do int. TE monitor applicably.
	//if(params->dsi.lcm_int_te_monitor)
	//	params->dsi.vertical_frontporch *= 2;

	// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.)
	//params->dsi.lcm_ext_te_monitor = FALSE;
	// Non-continuous clock
	//params->dsi.noncont_clock = TRUE;
	//params->dsi.noncont_clock_period = 2;	// Unit : frames

	// DSI MIPI Spec parameters setting
	/*params->dsi.HS_TRAIL = 6;
	params->dsi.HS_ZERO = 9;
	params->dsi.HS_PRPR = 5;
	params->dsi.LPX = 4;
	params->dsi.TA_SACK = 1;
	params->dsi.TA_GET = 20;
	params->dsi.TA_SURE = 6;
	params->dsi.TA_GO = 16;
	params->dsi.CLK_TRAIL = 5;
	params->dsi.CLK_ZERO = 18;
	params->dsi.LPX_WAIT = 1;
	params->dsi.CONT_DET = 0;
	params->dsi.CLK_HS_PRPR = 4;*/
	// Bit rate calculation
	params->dsi.PLL_CLOCK = 436;
}
#define GPIO_DISP_ID0_PIN  (GPIO124 | 0x80000000)
#define GPIO_DISP_ID1_PIN  (GPIO3 | 0x80000000)
static unsigned int lcm_compare_id(void)
{
	int pin_lcd_id0=0;
	int pin_lcd_id1=0;
	mt_set_gpio_mode(GPIO_DISP_ID0_PIN, 0);
	mt_set_gpio_dir(GPIO_DISP_ID0_PIN, 0);
	pin_lcd_id0=mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	
	mt_set_gpio_mode(GPIO_DISP_ID1_PIN, 0);
	mt_set_gpio_dir(GPIO_DISP_ID1_PIN, 0);
	pin_lcd_id1=mt_get_gpio_in(GPIO_DISP_ID1_PIN);
#ifdef BUILD_LK
	printf("hx8399c uboot %s\n", __func__);
#else
	printk("hx8399c kernel %s\n", __func__);
#endif
if((pin_lcd_id0==0x0)&&(pin_lcd_id1==0x0))
	return 1;
else
	return 0;
}

static void lcm_init(void)
{
	unsigned char cmd = 0x0;
	unsigned char data = 0xFF;
	int ret = 0;

	cmd = 0x00;
	data = 0x0E;
	SET_RESET_PIN(0);
	MDELAY(10);
	//lcm_util.set_gpio_out(GPIO_LCD_ENP, GPIO_OUT_ONE); 
	mt_set_gpio_out(GPIO_LCD_ENP, GPIO_OUT_ONE); 
	MDELAY(10);
	//lcm_util.set_gpio_out(GPIO_LCD_ENN, GPIO_OUT_ONE);
	mt_set_gpio_out(GPIO_LCD_ENN, GPIO_OUT_ONE);
	MDELAY(10);

#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = _lcm_i2c_write_bytes(cmd, data);
#endif
	if (ret < 0)
		LCM_PRINT("tps6132 cmd=%0x,ret=%d\n", cmd,ret);

	cmd = 0x01;
	data = 0x0E;

#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = _lcm_i2c_write_bytes(cmd, data);
#endif
	if (ret < 0)
		LCM_PRINT("tps6132 cmd=%0x,ret=%d\n", cmd,ret);
	
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
#ifdef BUILD_LK
	printf("hx8399c uboot %s\n", __func__);
#else
	printk("hx8399c kernel %s\n", __func__);
#endif
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{

	unsigned int data_array[10];
	

	data_array[0] = 0x00043902; // Display Off
	data_array[1] = 0x9983FFB9; // Sleep In	
	dsi_set_cmdq(data_array, 2, 1);
	
       data_array[0] = 0x00023902; // Display Off
	data_array[1] = 0x000000B1; // Sleep In	
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(5);	

	data_array[0] = 0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(100);

	//SET_RESET_PIN(0);
	//MDELAY(10);
	
	data_array[0] = 0x00043902; // Display Off
	data_array[1] = 0x9983FFB9; // Sleep In	
	dsi_set_cmdq(data_array, 2, 1);

	 data_array[0] = 0x00023902; // Display Off
	data_array[1] = 0x000001B1; // Sleep In	
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(5);	
	
//push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
	
	//lcm_util.set_gpio_out(GPIO_LCD_ENN, GPIO_OUT_ZERO);
	mt_set_gpio_out(GPIO_LCD_ENN, GPIO_OUT_ZERO);
	MDELAY(10);
	//lcm_util.set_gpio_out(GPIO_LCD_ENP, GPIO_OUT_ZERO); 
	mt_set_gpio_out(GPIO_LCD_ENP, GPIO_OUT_ZERO); 
	MDELAY(10);
#ifdef BUILD_LK
	printf("hx8399c  uboot %s\n", __func__);
#else
	printk("hx8399c  kernel %s\n", __func__);
#endif
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("hx8399c  uboot %s\n", __func__);
#else
	printk("hx8399c kernel %s\n", __func__);
#endif
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
	lcm_init();
}

#ifdef LCM_DSI_CMD_MODE
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

#ifdef BUILD_LK
	printf("uboot %s\n", __func__);
#else
	printk("kernel %s\n", __func__);
#endif

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}
#endif

static struct LCM_setting_table lcm_backlight_level_setting[] = {	
	{0X51, 2, {0x0f,0xff}},	//Write Display Brightness Value
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void lcm_setbacklight(unsigned int level)
{
unsigned int low_level,high_level=0;
static int bl_on = 0;
if(level==0)
{
	mt_set_gpio_out(GPIO_LCDBL_EN_PIN, GPIO_OUT_ZERO);
	bl_on = 0;
	//lcm_util.set_gpio_out(GPIO_LCDBL_EN_PIN, GPIO_OUT_ZERO);
}
else if(bl_on == 0)
{
	mt_set_gpio_out(GPIO_LCDBL_EN_PIN, GPIO_OUT_ONE);
	bl_on = 1;
}
	//add by yujianhua for minimum brightness
	if ( (level <= 5) && (level > 0))    
		level = 5;
	//add end
level=(unsigned int)level*3060/255;//4095 decrease the max brightness
       low_level=level&0xff;
	high_level=(level>>8)&0xff;
			  
       lcm_backlight_level_setting[0].para_list[0] = high_level;
	lcm_backlight_level_setting[0].para_list[1] = low_level;
#ifdef BUILD_LK
	printf("[LCD] lk hx8399c lcm_setbacklight level=%x,%x \n",low_level,high_level);
#else
	printk("[LCD] kernel  hx8399c lcm_setbacklight level=%x, %x \n",low_level,high_level);
#endif
	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);

}

LCM_DRIVER hx8399c_fhd_lead_dsi_vdo_drv = {
	.name           = "hx8399c_fhd_lead_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id     = lcm_compare_id,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.set_backlight = lcm_setbacklight,
#if defined(LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
};
