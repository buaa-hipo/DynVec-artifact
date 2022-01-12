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
            # y = ax + b
            _a = (y1[i+1]-y1[i]) / (x1[i+1]-x1[i])
            _b = (y1[i]*x1[i+1]-y1[i+1]*x1[i])/(x1[i+1]-x1[i])
            _y = _a*_x + _b
            res.append((_x,_y))
    return res

def to_M(x):
    return "{:.2}".format(x)

def draw(gather_log, load_log, fig_fn):
    fig ,ax = pyplot.subplots()
    ax.set_ylabel("Bandwidth (GB/s)", fontsize=12)
    ax.set_xlabel('Number of floating point values (1e6)', fontsize=12)
    ax2 = fig.add_axes([0.6, 0.6, 0.25, 0.25])
    ax2.set_xscale('log', basex=2)

    x1, y1 = read_data(gather_log)
    ax.plot( [ x/1e6 for x in x1], y1, label="gather" )
    ax2.plot(x1[:LOG_X_SUB], y1[:LOG_X_SUB])

    x2, y2 = read_data(load_log)
    ax.plot( [ x/1e6 for x in x1], y2, label="load" )
    ax2.plot(x2[:LOG_X_SUB], y2[:LOG_X_SUB])

    annot = find_lower(x1,y1,x2,y2)
    for pt in annot:
        if pt[0] < x1[LOG_X_SUB]:
            ax2.plot([pt[0],pt[0]],[0,pt[1]],linestyle='--')
            #ax2.text(pt[0]+100, 0, pt[0])
        else:
            ax.plot([pt[0]/1e6,pt[0]/1e6],[0,pt[1]],linestyle='--')
            ax.text(pt[0]/1e6, 0, to_M(pt[0]/1e6))
    ax.legend(loc='lower right')
    fig.savefig(fig_fn, bbox_inches = 'tight')
    fig.savefig(fig_fn+'.png', bbox_inches = 'tight')

def draw_all(gather_log, load_log, fig_fn):
    fig ,ax = pyplot.subplots()
    ax2 = fig.add_axes([0.25, 0.6, 0.25, 0.25])
    ax.set_ylabel("Speedup", fontsize=14)
    ax.set_xlabel('Number of floating point values (1e6)', fontsize=14)
    _y_1 = None
    _y_1_s = None
    for i in range(0, len(gather_log)):
        x1, y1 = read_data(gather_log[i])
        x2, y2 = read_data(load_log[i])
        y = [ y2[i]/y1[i] for i in range(0, len(y1)) ]
        ax.plot([x/1e6 for x in x1], y, label=str(i+1)+' L/G')
        ax2.plot(x1[:LOG_X_SUB], y[:LOG_X_SUB])
        if i == 0:
            _y_1 = [ 1 for i in x1 ]
            _y_1_s = [ 1 for i in x1[:LOG_X_SUB] ]
            ax.plot([x/1e6 for x in x1], _y_1, linestyle='--', color='red')
            ax2.plot(x1[:LOG_X_SUB], _y_1_s, linestyle='--', color='red')
        annot = find_lower(x1, y, x1, _y_1)
        for pt in annot:
            if pt[0] < x1[LOG_X_SUB]:
                ax2.plot([pt[0],pt[0]],[0,pt[1]],linestyle='--')
                #ax2.text(pt[0]+100, 0, pt[0])
            else:
                ax.plot([pt[0]/1e6,pt[0]/1e6],[0,pt[1]],linestyle='--')
                ax.text(pt[0]/1e6, 0, to_M(pt[0]/1e6))

    ax.legend()
    fig.savefig(fig_fn, bbox_inches = 'tight')
    fig.savefig(fig_fn+'.png', bbox_inches = 'tight')

NUM=4
# DATA_FOLD="data-gold/data-single/"
# #FIG_FOLD="motivation-single-512/"
# FIG_FOLD="motivation-single-gold-512/"

DATA_FOLD="data-haswell/data-double/"
FIG_FOLD="motivation-double-haswell-256/"

# DATA_FOLD="data-gold/data-double/"
# FIG_FOLD="motivation-double-gold-512/"

# DATA_FOLD="data-knl/data-double/"
# FIG_FOLD="motivation-double-knl-512/"

# DATA_FOLD="data-knl/data-single/"
# FIG_FOLD="motivation-single-knl-512/"

if not os.path.exists(FIG_FOLD):
    os.mkdir(FIG_FOLD)

gather_all = [ DATA_FOLD+'gather_'+str(x)+'.dat' for x in range(1, NUM+1) ]
load_all = [ DATA_FOLD+'load_'+str(x)+'.dat' for x in range(1, NUM+1) ]
for i in range(0, NUM):
    draw(gather_all[i], load_all[i], FIG_FOLD+str(i+1)+".pdf")
draw_all(gather_all, load_all, FIG_FOLD+"speedup.pdf")


# pyplot.show()