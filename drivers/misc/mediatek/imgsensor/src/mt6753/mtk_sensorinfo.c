/*
 * 
 *
 * For sensor cfg test
 *
 * Copyright (C) 2009-2014 ZFG Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/semaphore.h>
#define MAX_NAME_LENGTH     32
#define SENSOR_INFO_OV13853_MODEL_ID    0xD853
#define SENSOR_INFO_S5K5E2_MODEL_ID     0x5e20
static uint16_t module_integrator_id = 0;

static uint16_t g_back_sensor_id = 0;
static uint16_t g_front_sensor_id = 0;

DEFINE_SEMAPHORE(set_back_sensor_id_sem);
void mtk_sensorinfo_set_back_sensor_id(uint16_t id)
{
    down(&set_back_sensor_id_sem);
    g_back_sensor_id = id;
    up(&set_back_sensor_id_sem);
}
EXPORT_SYMBOL(mtk_sensorinfo_set_back_sensor_id);


DEFINE_SEMAPHORE(set_back_sensor_module_id_sem);
void mtk_sensorinfo_set_back_sensor_module_id(uint16_t module_id)
{
    down(&set_back_sensor_module_id_sem);
	module_integrator_id = module_id;
    up(&set_back_sensor_module_id_sem);
}
EXPORT_SYMBOL(mtk_sensorinfo_set_back_sensor_module_id);


DEFINE_SEMAPHORE(set_front_sensor_id_sem);
void mtk_sensorinfo_set_front_sensor_id(uint16_t id)
{
    down(&set_front_sensor_id_sem);
    g_front_sensor_id = id;
    up(&set_front_sensor_id_sem);   
}
EXPORT_SYMBOL(mtk_sensorinfo_set_front_sensor_id);


static int rear_camera_id_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%x\n", g_back_sensor_id);
	return 0;
}
static int rear_camera_name_proc_show(struct seq_file *m, void *v)
{
	char sensor_name[MAX_NAME_LENGTH] = {0x00};	
	switch(g_back_sensor_id)
	{
		case SENSOR_INFO_OV13853_MODEL_ID:			
			  sprintf(sensor_name, "OV13853-13M-AF");
			break;
		default:
			sprintf(sensor_name, "No sensor or error ID!");
			break;
	}
	seq_printf(m, "%s\n", sensor_name);
	return 0;
}

static int front_camera_id_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%x\n", g_front_sensor_id);
	return 0;
}
static int front_camera_name_proc_show(struct seq_file *m, void *v)
{
	char sensor_name[MAX_NAME_LENGTH] = {0x00};	
	switch(g_front_sensor_id)
	{
		case SENSOR_INFO_S5K5E2_MODEL_ID :			
			  sprintf(sensor_name, "S5K5E2-5M-FF");
			break;
		default:
			sprintf(sensor_name, "No sensor or error ID!");
			break;
	}
	seq_printf(m, "%s\n", sensor_name);
	return 0;
}


static int rear_camera_id_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rear_camera_id_proc_show, NULL);
}
static int rear_camera_name_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rear_camera_name_proc_show, NULL);
}

static int front_camera_id_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, front_camera_id_proc_show, NULL);
}
static int front_camera_name_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, front_camera_name_proc_show, NULL);
}


static const struct file_operations rear_camera_id_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= rear_camera_id_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static const struct file_operations rear_camera_name_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= rear_camera_name_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static const struct file_operations front_camera_id_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= front_camera_id_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static const struct file_operations front_camera_name_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= front_camera_name_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
extern const struct file_operations flash_proc_fops;
/*
 * Attention:
 *
 * 
 */
static int __init sensorinfo_init(void)
{	
	struct proc_dir_entry *dir, *res;

	dir = proc_mkdir("camera", NULL);
	if (!dir)
		return -ENOMEM;

	res = proc_create_data("rear_camera_id", 0, dir,
			       &rear_camera_id_proc_fops, NULL);
	if (!res)
		return -ENOMEM;

	res = proc_create_data("rear_camera_name", 0, dir,
			       &rear_camera_name_proc_fops, NULL);
	if (!res)
		return -ENOMEM;

	res = proc_create_data("front_camera_id", 0, dir,
			       &front_camera_id_proc_fops, NULL);
	if (!res)
		return -ENOMEM;

	res = proc_create_data("front_camera_name", 0, dir,
			       &front_camera_name_proc_fops, NULL);
	if (!res)
		return -ENOMEM;
  return 0;
}
module_init(sensorinfo_init);
