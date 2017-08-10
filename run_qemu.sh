#!/bin/bash

BR2_PATH="buildroot-2017.05.2/"
qemu-system-arm -M vexpress-a9 -smp 1 -m 256 -display none -kernel "$BR2_PATH"output/images/zImage -dtb "$BR2_PATH"output/images/vexpress-v2p-ca9.dtb -drive file="$BR2_PATH"output/images/rootfs.ext2,if=sd,format=raw -append "console=ttyAMA0,115200 root=/dev/mmcblk0" -serial stdio -net nic,model=lan9118 -net user

