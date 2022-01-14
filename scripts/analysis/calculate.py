#! /usr/bin/python
import os
import re
from sys import argv
path_log = argv[1]

log_icc = open(path_log+"/icc.dat")
log_csr5 = open(path_log+"/csr5.dat")
log_mkl = open(path_log+"/mkl.dat")
log_our = open(path_log+"/dynvec.dat")

def read_data( func_map , lines ):
    for line in lines:
        data2 = line.split()
        name = data2[0].strip()
        gflops = float(data2[1].strip())
        func_map[name] = gflops

map_csr5 = dict()
map_our = dict()
map_mkl = dict()
map_icc = dict()
read_data(map_icc,log_icc.readlines())

read_data(map_csr5,log_csr5.readlines())
read_data(map_mkl,log_mkl.readlines())
read_data(map_our,log_our.readlines())
csr5_i = 0
icc_i = 0
mkl_i = 0
map_our_num = len(map_our)
csr5_speed_up = 0
icc_speed_up = 0
mkl_speed_up = 0

map_all_num = 0
print("Total DynVec num:", map_our_num)

for (name,gflops) in map_our.items():
    if name in map_csr5 and name in map_mkl:
        if gflops > map_csr5[name]:
            csr5_i = csr5_i + 1
        if gflops > map_icc[name]:
            icc_i = icc_i + 1
        if gflops > map_mkl[name]:
            mkl_i = mkl_i + 1
        csr5_speed_up = csr5_speed_up + gflops / map_csr5[name]
        icc_speed_up = icc_speed_up + gflops / map_icc[name]
        mkl_speed_up = mkl_speed_up + gflops / map_mkl[name]
        map_all_num = map_all_num + 1

print("Total num:", map_all_num)
print('csr5', csr5_i , csr5_speed_up / map_all_num)
print('mkl', mkl_i , mkl_speed_up / map_all_num)
print('icc', icc_i , icc_speed_up / map_all_num)


