#! /usr/bin/python
import os
from sys import argv
path=argv[1]
files = os.listdir(path)
for file in files:
    execute_file = path + "/" + file
    execute_file = "./spmv " +  execute_file
    #print(execute_file)
    os.system( execute_file + ">>log_spmv_ourmethod2" )
