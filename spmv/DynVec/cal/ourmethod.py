#! /usr/bin/python
import os
import re
from sys import argv
path_log = argv[1]

begin = "-----"
log_file = open(path_log)
lines = log_file.readlines()
result = []
size_lines = len(lines)
new_result = [lines[0] ]
i = 1
while i < size_lines:
    while i < size_lines : 
        if( lines[i][0] == '-' and lines[i][1] == '-' ):
            result.append(new_result)
            new_result = [lines[i]]
            i = i + 1
            break
        else:
            new_result.append( lines[i] )
        i = i + 1

result.append(new_result)
i = 0
for res in result:
    if( res[len(res)-1] == "Correct\n"):
        file_name = re.findall(r"mx_data_less_than_1G//(.+).mtx", res[0])
        
        gflops = re.findall(r"jit GFLOPS: (.+) count:", res[len(res)-2])
        print file_name[0],gflops[0]
        i = i + 1
     
