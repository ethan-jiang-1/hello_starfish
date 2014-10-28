# User specific aliases and functions
XTENSA_CORE=KF1_prod_rel_2012_4
XTENSA_TOOLS_ROOT=/cad/tensilica/tools/RD-2012.4-linux/XtensaTools
XTENSA_ROOT=/cad/tensilica/chips/kingfisher/RD-2012.4-linux/$XTENSA_CORE

XTENSA_SYSTEM=$XTENSA_ROOT/config

LM_LICENSE_FILE=/home/zg/license.dat

ifconfig eth0 down
ifconfig eth0 hw ether 00:0c:29:01:02:03
ifconfig eth0 up

PATH=$PATH:$XTENSA_TOOLS_ROOT/bin

export LM_LICENSE_FILE XTENSA_TOOLS_ROOT XTENSA_ROOT XTENSA_SYSTEM XTENSA_CORE PATH

export XTENSA_PREFER_LICENSE=XT-GENERIC

export KF_SCS_DIR=`pwd`
export TARGET=QCA4004
export FLAVOR=SP143
export AR6002_REV=7
export AR6002_REV7_VER=2
export FPGA_FLAG=0
export THREADX_EN=1
export SSL_EN=0
export ALLJOYN_EN=0


#prepare for make flash image
make
cp image/* .output/QCA4004/SP143/image/
rm -rf output
mkdir output
cp .output/QCA4004/SP143/image/sdk_flash.out output/sdk_app.out
cd flash
. R_3_1_app_flash.sh
cd ../
