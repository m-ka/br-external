#!/usr/bin/env python

import os, sys
import random

STORAGE_FILE = "/dev/barx-storage"
SYSFS_READ_CNTR = "/sys/devices/platform/barx-storage/barx/read_cntr"
SYSFS_WRITE_CNTR = "/sys/devices/platform/barx-storage/barx/write_cntr"

def test_zero():
    """
    Write zeros to storage and check if the data has been written
    """
    size = 0
    fd = os.open(STORAGE_FILE, os.O_RDWR)
    size = os.lseek(fd, 0, os.SEEK_END)
    os.lseek(fd, 0, os.SEEK_SET)
    w_data = bytearray([0 for i in range(0, size)])
    os.write(fd, w_data)
    os.close(fd)

    fd = os.open(STORAGE_FILE, os.O_RDONLY)
    r_data = os.read(fd, size)
    for i in range(0, size):
        if r_data[i] != chr(0):
            return -1
    return 0

def test_counter():
    """
    Write counter value to storage and check if the data has been written
    """
    size = 0
    fd = os.open(STORAGE_FILE, os.O_RDWR)
    size = os.lseek(fd, 0, os.SEEK_END)
    os.lseek(fd, 0, os.SEEK_SET)
    w_data = bytearray([i % 256 for i in range(0, size)])
    os.write(fd, w_data)
    os.close(fd)

    fd = os.open(STORAGE_FILE, os.O_RDONLY)
    r_data = os.read(fd, size)
    for i in range(0, size):
        if r_data[i] != chr(i % 256):
            return -1

    for i in range(0, 10):
        pos = random.randrange(size)
        os.lseek(fd, pos, os.SEEK_SET)
        r_data = os.read(fd, 1)
        if r_data[0] != chr(pos % 256):
            print("Error: lseek is set incorrectly")
            return -1
    os.close(fd)

    return 0

def get_cntr(f_name):
    """
    Get data from sysfs attribute
    """
    fd = os.open(f_name, os.O_RDONLY)
    cntr = os.read(fd, 10)
    return cntr

def get_counters():
    """
    Print values of all counters
    """
    r_cntr = get_cntr(SYSFS_READ_CNTR)
    w_cntr = get_cntr(SYSFS_WRITE_CNTR)
    print("Read counter: {0}".format(r_cntr))
    print("Write counter: {0}".format(w_cntr))

if __name__ == "__main__":
    get_counters()
    if test_zero() < 0:
        res = "failed"
    else:
        res = "passed"
    print("Zero values test {0}".format(res))

    get_counters()
    if test_counter() < 0:
        res = "failed"
    else:
        res = "passed"
    print("Counter test {0}".format(res))

