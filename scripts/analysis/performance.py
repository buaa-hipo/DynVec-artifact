#! /usr/bin/python
import os
import re
from sys import argv
import matplotlib.pyplot as plt
import scipy.stats as st
import numpy as np

def extract_data(path_log):
    log_icc = open(path_log+"/log_icc.dat")
    log_our = open(path_log+"/log_dynvec.dat")

    def read_data( func_map , lines ):
        for line in lines:
            data2 = line.split()
            name = data2[0].strip()
            gflops = float(data2[1].strip())
            func_map[name] = gflops

    map_icc = dict()
    map_our = dict()
    read_data(map_icc,log_icc.readlines())
    read_data(map_our,log_our.readlines())
    icc_i = 0
    map_our_num = len(map_our)
    icc_speed_up = 0

    icc_speed_up_eff = 0

    speedup_all = {
        "icc" : []
    }

    map_all = {
        "icc" : {},
        "dynvec" : {},
        "max" : {}
    }

    map_all_num = 0
    print("Total DynVec num:", map_our_num)
    icc_avg = 0
    for (name,gflops) in map_our.items():
        if name in map_icc:
            if gflops > map_icc[name]:
                icc_i = icc_i + 1
                icc_avg = icc_avg + float(gflops/map_icc[name])
                icc_speed_up_eff = icc_speed_up_eff + gflops / map_icc[name]
            speedup_all['icc'].append(gflops / map_icc[name])
            icc_speed_up = icc_speed_up + gflops / map_icc[name]
            map_all_num = map_all_num + 1
            map_all['icc'][name] = map_icc[name]
            map_all['dynvec'][name] = gflops
            map_all['max'][name] = max([gflops, map_icc[name]])

    print(f"better that icc num = {icc_i}, average speedup = {icc_avg/icc_i}")
    return map_all, speedup_all

def extract_oi():
    log_args = open("log_args", "r")
    arg_info = {
        "m" : {},
        "n" : {},
        "nnz" : {},
        "bytes" : {},
        "OI" : {}
    }
    for line in log_args.readlines():
        data2 = line.split()
        name = data2[0].strip()
        m = int(data2[1].strip())
        n = int(data2[2].strip())
        nnz = int( data2[3].strip() )
        arg_info['m'][ name ] = m
        arg_info['n'][ name ] = n
        arg_info['nnz'][ name ] = nnz
        arg_info['bytes'][name] = nnz*(8+4+8)+m*(8+4)+4
        arg_info['OI'][name] = (nnz*2) / (nnz*(8+4+8)+m*(8+4)+4)
    return arg_info

def better_bar_demo(bars):
    bar_num = len(bars[0])
    bar_coll_num = len(bars)
    print("bar collection num: {bcn}, bar num: {bn}.".format(bcn=bar_coll_num, bn=bar_num))
    for i in range(1, bar_coll_num):
        assert(len(bars[i]) == bar_num)
    for k in range(0, bar_num):
        tmp = [ None for i in range(0, bar_coll_num) ]
        for i in range(0, bar_coll_num):
            tmp[i] = bars[i][k]
        i = bar_coll_num + 10
        for bar in sorted(tmp, key=lambda rect: rect.get_height()):
            bar.set(zorder=i)
            i = i - 1

def draw_roofline_bar(ax, map_all, arg_info, bandwidth, x_notes, show_y, show_legend):
    data = {
        'icc' : [],
        'dynvec' : []
    }
    print('MAX: ',sorted(map_all['max'].items(), key=lambda item: item[1])[-1])
    for name, roof in sorted(map_all['max'].items(), key=lambda item: item[1]):
        data['icc'].append(map_all['icc'][name])
        data['dynvec'].append(map_all['dynvec'][name])
    x = list(range(0, len(data['dynvec'])))
    bars = []
    bar = ax.bar(x, data['icc'], alpha=0.5, color='green', label='GCC')
    bars.append(bar)
    bar = ax.bar(x, data['dynvec'], alpha=0.5, color='red', label='DynVec')
    bars.append(bar)
    ax.set_ylim(0, 8)
    
    better_bar_demo(bars)
    
    if show_y:
        ax.set_ylabel('Performance (GFlops/s)')
    ax.set_xlabel('Matrix ID' + '\n' + x_notes)
    if show_legend:
        ax.legend()

def read_bandwidth_data(log):
    x_data = []
    y_data = []
    with open(log) as f:
        for line in f:
            cont = line.split(" ")
            x = int(cont[0]) 
            y = float(cont[1])
            x_data.append(x)
            y_data.append(y)
    return x_data, y_data

if __name__ == '__main__':
    arg_info = extract_oi()
    plt.rcParams['figure.figsize'] = (7.0, 2.5)
    plt.rc('font', **{'size': 11})
    fig, ax = plt.subplots(1,1, sharey=True)
    fig.subplots_adjust(hspace=0, wspace=0)
    i = 0
    prefix = '(a)'
    platform = 'ARM'
    path = str(argv[1])
    print(path)
    map_all, speedup_all = extract_data(path)
    draw_roofline_bar(ax, map_all, arg_info, 0, prefix+' '+platform, True, True)
    plt.tight_layout()
    plt.savefig('performance.pdf')
    plt.close()