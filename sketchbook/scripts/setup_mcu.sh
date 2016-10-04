#! /bin/bash

_programmer=dragon_isp
_mcu=m328p
_port=usb
_bootloader=optiboot_atmega328.hex
_efuse=0x05
_lfuse=0xFF
_hfuse=0xDE

_delay=2

function bootload() {
    sudo avrdude -c $_programmer -p $_mcu -P $_port -F -U flash:w:$_bootloader
}

function fuse() {
    sudo avrdude -c $_programmer -p $_mcu -P $_port -F -U $1:w:$2:m
}

case "$1" in
    "bootload")
        bootload
        ;;
    "hfuse")
        fuse hfuse $_hfuse
        ;;
    "lfuse")
        fuse lfuse $_lfuse
        ;;
    "efuse")
        fuse efuse $_efuse
        ;;
    "all")
        bootload
        sleep $_delay
        fuse hfuse $_hfuse
        sleep $_delay
        fuse lfuse $_lfuse
        sleep $_delay
        fuse efuse $_efuse
        ;;
esac
echo "--DONE--"
