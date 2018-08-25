#!/usr/bin/python

from ctypes import cdll

def callc():
    somelibc = cdll.LoadLibrary("/home/admin/tiechou/testLIB/libhello.so")
    somelibc.hello()
if __name__=="__main__":
    callc()
