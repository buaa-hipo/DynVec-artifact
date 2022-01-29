#! /usr/bin/python
import os
import re
from sys import argv
path_log = argv[1]
begin0 = "------------------------------------------------------\n"
begin1 = "PRECISION = 64-bit Double Precision\n"

begin2 = "------------------------------------------------------\n"
log_file = open(path_log)
lines = log_file.readlines()
result = []
size_lines = len(lines)
i = 3
new_result = []
while i < size_lines:
    while i < size_lines : 
        if( i + 3 < size_lines and lines[i] == begin0 
            and lines[i+1 ] == begin1 
            and lines[i+2] == begin2 ):
            result.append(new_result)
            new_result = []
            i = i + 3
            break
        else:
            new_result.append( lines[i] )
        i = i + 1

result.append(new_result)

i = 0
for res in result:

    if( res[len(res)-2] == "Check... PASS!\n"):
        file_name = re.findall(r"mx_data_less_than_1G/(.+).mtx", res[0])
        
        gflops = re.findall(r"GFlops = (.+) GFlops", res[len(res)-3])
        print file_name[0],gflops[0]
        i = i + 1

