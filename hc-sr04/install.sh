#!/bin/sh
make -C ~/linux-3.16.1 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- M=`pwd` modules
sshpass -p acmesystems scp hcsr04.ko root@192.168.10.10:/root
sshpass -p acmesystems scp hcsr04_est.ko root@192.168.10.10:/root
sshpass -p acmesystems scp hcsr04_ovest.ko root@192.168.10.10:/root
