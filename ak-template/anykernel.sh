### AnyKernel3 Samsung Galaxy A7 2017 Exynos7880 (KSU+SUSFS)

properties() { '
kernel.string=A7 2017 KSU+SUSFS Kernel by TomKun
do.devicecheck=0
do.modules=0
do.systemless=1
do.cleanup=1
do.cleanuponabort=0
device.name1=a7y17lte
device.name2=a7y17ltekor
device.name3=a720f
'; }

BLOCK=/dev/block/platform/13540000.dwmmc0/by-name/BOOT;
IS_SLOT_DEVICE=0;
RAMDISK_COMPRESSION=auto;
PATCH_VBMETA_FLAG=auto;

. tools/ak3-core.sh;

dump_boot;
write_boot;
