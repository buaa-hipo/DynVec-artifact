#! /usr/bin/python
import matplotlib as plt
import numpy as np
import matplotlib.pyplot as pyplot
import math
import os

X_SUB = (1024*1024)/16
LOG_X_SUB = int(math.log2(X_SUB))

def read_data(log):
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

def find_lower(x1, y1, x2, y2):
    res = []
    for i in range(0, len(x1)-1):
        if (y1[i] <= y2[i] and y1[i+1] >= y2[i+1]) or (y1[i] >= y2[i] and y1[i+1] <= y2[i+1]):
            _y1 = y2[i] - y1[i]
            _y2 = y1[i+1] - y2[i+1]
            _x = x1[i]+(x1[i+1]-x1[i])*(_y1/(_y1+_y2))
            _a = (y1[i+1]-y1[i]) / (x1[i+1]-x1[i])
            _b = (y1[i]*x1[i+1]-y1[i+1]*x1[i])/(x1[i+1]-x1[i])
            _y = _a*_x + _b
            res.append((_x,_y))
    return res

def to_M(x):
    return "{:.2}".format(x)

def draw_all(ax, is_DP, show_y, show_legend, gather_log, load_log):
    if show_y:
        if is_DP:
            ax.set_ylabel("(i) gather (DP)\nSpeedup", fontsize=10)
        else:
            ax.set_ylabel("(ii) gather (SP)\nSpeedup", fontsize=10)
    ax.set_xscale('log', base=2)
    ax.set_yscale('log', base=2)
    _y_1 = None
    _y_1_s = None
    for i in range(0, len(gather_log)):
        x1, y1 = read_data(gather_log[i])
        x2, y2 = read_data(load_log[i])
        y = [ y2[i]/y1[i] for i in range(0, len(y1)) ]
        if i+1 not in [1, 2, 4, 8]:
            continue
        ax.plot([x for x in x1], y, label=str(i+1)+' LPB')
        if i==0:
            print("{0} LPB effective speedup: {1}".format(i+1, np.mean(list(filter(lambda a: a>1, y)))))
        if i == 0:
            _y_1 = [ 1 for i in x1 ]
            _y_1_s = [ 1 for i in x1[:LOG_X_SUB] ]
            ax.plot([x for x in x1], _y_1, linestyle='--', color='red')
    if show_legend:
        ax.legend(loc=(-1.9,0.85), ncol=4)

def draw_scatter(ax, show_y, show_legend, scatter_all, plat, x_plat):
    if show_y:
        ax.set_ylabel("(iii) scatter\nSpeedup", fontsize=10)
    ax.set_xlabel('Array Size (elements)'+x_plat, fontsize=10)
    ax.set_xscale('log', base=2)
    ax.set_yscale('log', base=2)
    _y_1 = None
    _y_1_s = None
    ax.plot(scatter_all['double']['speedup'][0], scatter_all['double']['speedup'][1], label='DP', color='blue')
    ax.plot(scatter_all['single']['speedup'][0], scatter_all['single']['speedup'][1], label='SP', color='black')
    ax.plot([32, max(scatter_all['double']['speedup'][0])], [1, 1], linestyle='--', color='red')
    if show_legend:
        ax.legend(loc=(-1.05,0.05), ncol=2)

data_list = [
    "data/data-double/",
    "data/data-single/",
]

fig_list = [
    "figures/double-kp-512/",
    "figures/single-kp-512/",
]

num_list = [
    4, 8
]

pyplot.rc('font', **{'size': 10})
pyplot.rcParams['figure.figsize'] = (10.0, 4.0)

fig ,ax = pyplot.subplots(3, 1, sharex=True, sharey=True)
fig.subplots_adjust(hspace=0, wspace=0)
for i in range(len(num_list)):
    if i>=8:
        break
    print(i%2, i, data_list[i])
    NUM=num_list[i]
    DATA_FOLD=data_list[i]

    gather_all = [ DATA_FOLD+'gather_'+str(x)+'.dat' for x in range(1, NUM+1) ]
    load_all = [ DATA_FOLD+'load_'+str(x)+'.dat' for x in range(1, NUM+1) ]
    draw_all(ax[i%2], i%2==0, i<2, i==5, gather_all, load_all)

platform = [ 'kunpeng']
x_plat = ['\n(d) KP920b']
k = 0
for p in platform:
    DATA_FOLD_DP='data/data-double/'
    DATA_FOLD_SP='data/data-single/'

    scatter_all = {
        'double' : {
            'scatter' : read_data(DATA_FOLD_DP+'scatter_pure_1.dat'),
            'store' : read_data(DATA_FOLD_DP+'store_1.dat')
        },
        'single' : {
            'scatter' : read_data(DATA_FOLD_SP+'scatter_pure_1.dat'),
            'store' : read_data(DATA_FOLD_SP+'store_1.dat')
        }
    }
    scatter_all['double']['speedup'] = (scatter_all['double']['scatter'][0], 
                                        [ scatter_all['double']['store'][1][i]/scatter_all['double']['scatter'][1][i] 
                                            for i in range(0, len(scatter_all['double']['store'][1])) ] )
    scatter_all['single']['speedup'] = (scatter_all['single']['scatter'][0], 
                                        [ scatter_all['single']['store'][1][i]/scatter_all['single']['scatter'][1][i] 
                                            for i in range(0, len(scatter_all['single']['store'][1])) ] )
    draw_scatter(ax[2], k==0, k==2, scatter_all, p, x_plat[k])
    k = k + 1
fig.savefig('motivation_exp.pdf', bbox_inches = 'tight')