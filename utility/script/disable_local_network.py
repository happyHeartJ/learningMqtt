# -*- coding:utf-8 -*-

import os
import subprocess
import threading
import random
import time

__author__ = "JiangHongJun"
__doc__ = "The script is used for testing the app based on libevent.\
Disable and enable the local network in turn."

flag = False


def pycmd():
    global flag
    if flag == False:
        print('disabled local network')
        os.system('netsh interface set interface "本地连接" disabled')
        flag = True
    else:
        print('enabled local network')
        os.system('netsh interface set interface "本地连接" enabled')
        time.sleep(10)
        flag = False


def sayhello():
    # print("hello world")
    pycmd()
    global t  # Notice: use global variable!
    global timeInterval
    timeInterval = random.randint(10, 15)
    print('the new time interval is %d seconds' % timeInterval)
    t = threading.Timer(timeInterval, sayhello)
    t.start()

timeInterval = random.randint(10, 15)
t = threading.Timer(timeInterval, sayhello)
t.start()

if __name__ == '__main__':
    print('Start python script to disable and enable the local network...')
    sayhello
