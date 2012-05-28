#!/bin/bash

# To use this script you need to setup the `WRS_OUTPUT_DIR` variable by doing:
# 
# 	export WRS_OUTPUT_DIR=/path/to/wrs/output/dir		
# 
# You should call the script like this:
# 		
# 		./build.sh wrs3_defconfig
# 		./build.sh all


make LINUX=${WRS_OUTPUT_DIR}/build/linux-2.6.39/ CROSS_COMPILE=${WRS_OUTPUT_DIR}/build/buildroot-2011.11/output/host/usr/bin/arm-linux- ARCH=arm $1
