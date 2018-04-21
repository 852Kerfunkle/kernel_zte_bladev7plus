/*
 * Driver for CAM_CAL
 *
 *
 */

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
extern void kdSetI2CSpeed(u16 i2cSpeed);

#define PFX "ov13853_OTP"
#define LOG_INF(format, args...)    pr_err(PFX "[%s] " format, __FUNCTION__, ##args)

#define USHORT             unsigned short
#define BYTE               unsigned char
#define Sleep(ms) mdelay(ms)

#define EEPROM           GT24C64
#define EEPROM_READ_ID   0xA1
#define EEPROM_WRITE_ID  0xA0
#define I2C_SPEED        400  //CAT24C512 can support 1Mhz

#define START_OFFSET     0
#define PAGE_NUM         512
#define EEPROM_PAGE_SIZE 128  //EEPROM size 512x128=65536bytes
#define MAX_OFFSET       0xffff
#define DATA_SIZE 2048
BYTE ov13853_eeprom_data[DATA_SIZE]= {0};
static bool get_done = false;
static int last_size = 0;
static int last_offset = 0;

bool selective_read_eeprom(kal_uint16 addr, BYTE* data)
{
	char pu_send_cmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    if(addr > MAX_OFFSET)
        return false;
	kdSetI2CSpeed(I2C_SPEED);

	if(iReadRegI2C(pu_send_cmd, 2, (u8*)data, 1, EEPROM_READ_ID)<0)
		return false;
    return true;
}

bool OV13853_read_eeprom(kal_uint16 addr, BYTE* data, kal_uint32 size ){
	int i = 0;
	//int offset = addr;
	int offset = 0x77A;
	for(i = 0; i < 496; i++) {
		if(!selective_read_eeprom(offset, &data[i])){
			LOG_INF("read_eeprom 0x%0x %d fail \n",offset, data[i]);
			return false;
		}
		LOG_INF("read_eeprom 0x%0x 0x%x\n",offset, data[i]);
		offset++;
	}

	offset = 0x96c;
	for(i = 496; i < 496+876; i++) {
		if(!selective_read_eeprom(offset, &data[i])){
			LOG_INF("read_eeprom 0x%0x %d fail \n",offset, data[i]);
			return false;
		}
		LOG_INF("read_eeprom 0x%0x 0x%x\n",offset, data[i]);
		offset++;
	}
	get_done = true;
	last_size = size;
	last_offset = addr;
    return true;
}

bool read_otp_pdaf_data( kal_uint16 addr, BYTE* data, kal_uint32 size){
	
	LOG_INF("read_otp_pdaf_data enter");
    size = 1372;
	
	if(!get_done || last_size != size || last_offset != addr) {
		if(!OV13853_read_eeprom(addr, ov13853_eeprom_data, size)){
			get_done = 0;
            last_size = 0;
            last_offset = 0;
			LOG_INF("read_otp_pdaf_data fail");
			return false;
		}
	}
	memcpy(data, ov13853_eeprom_data, size);
	LOG_INF("read_otp_pdaf_data end");
    return true;
}
