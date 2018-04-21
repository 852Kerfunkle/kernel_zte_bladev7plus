#
# Makefile for misc devices that really don't fit anywhere else.
#

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ARCH_MTK_PROJECT := $(subst ",,$(CONFIG_ARCH_MTK_PROJECT))

MTK_CUSTOM_PATH  := $(srctree)/drivers/misc/mediatek

ifneq ($(CONFIG_ARM64), y)
# For arm32, mach folder
MTK_PLATFORM_PATH_ROOT := $(srctree)/arch/arm/mach-$(MTK_PLATFORM)
MTK_PROJECT_PATH_ROOT := $(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)
  ifneq ($(filter y,$(CONFIG_ARCH_MT6752) $(CONFIG_ARCH_MT6732) $(CONFIG_ARCH_MT6595) $(CONFIG_ARCH_MT6795) $(CONFIG_ARCH_MT6735) $(CONFIG_ARCH_MT6735M) $(CONFIG_ARCH_MT6753) $(CONFIG_ARCH_MT6580)),)
    # For armv8 32bit
    MTK_PLATFORM_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)
    MTK_PROJECT_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)
  endif
else
# For arm64, no mach-xxx folder
MTK_PLATFORM_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)
MTK_PROJECT_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)
endif


subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/cam_cal/inc

# obj-$(CONFIG_MTK_OTP_CAT24C64_SUPPORT)    += common/cat24c64/
obj-y += cam_cal_drv.o
obj-y += cam_cal_list.o
obj-y += common/GT24c64a/GT24c64a.o
