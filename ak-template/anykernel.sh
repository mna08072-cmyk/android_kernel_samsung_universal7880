### AnyKernel3 Ramdisk Mod Script
## osm0sis @ xda-developers

### AnyKernel setup
# global properties
properties() { '
kernel.string=A7 2017 KSU+SUSFS Kernel by TomKun
do.devicecheck=0
do.modules=0
do.systemless=0
do.cleanup=1
do.cleanuponabort=0
supported.versions=
supported.patchlevels=
'; } # end properties

# shell variables
BLOCK=/dev/block/platform/13540000.dwmmc0/by-name/BOOT;
IS_SLOT_DEVICE=0;
RAMDISK_COMPRESSION=auto;
PATCH_VBMETA_FLAG=auto;

## AnyKernel methods (DO NOT CHANGE)
# import patching functions/variables - see for reference
. tools/ak3-core.sh;

## AnyKernel boot install
dump_boot;

vbmeta_disable_verification;

write_boot;
## end boot install
