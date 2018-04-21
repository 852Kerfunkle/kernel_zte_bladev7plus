/*****************************************************************************
 *
 * Filename:
 * ---------
 *   S-24CS64A.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Header file of EEPROM driver
 *
 *
 * Author:
 * -------
 *   Ronnie Lai (MTK01420)
 *
 *============================================================================*/
#ifndef __GT24C64A_H
#define __GT24C64A_H

//#define EEPROM_DEV_MAJOR_NUMBER 226

extern unsigned int gt24c32a_selective_read_region(struct i2c_client *client, unsigned int addr, unsigned char *data, unsigned int size);

#endif /* __EEPROM_H */

