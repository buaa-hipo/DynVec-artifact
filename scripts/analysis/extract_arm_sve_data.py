#! /usr/bin/python3
import os
import re
from sys import argv

def extract_dynvec(path_log):
    data = {
        'name' : [],
        'jit'  : [],
        'aot'  : [],
        'jit time'  : [],
        'aot time'  : [],
        'llvmcompile time' : [],
        'compile time' : [],
        'overhead' : []
    }
    begin = "-----"
    log_file = open(path_log, 'r')
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
            file_name = re.findall(r"mx_data_less_than_1G/(.+).mtx", res[0])
            data['name'].append(file_name[0])
            llvm_ct = re.findall(r"llvmcompile Time: (.+) count:", res[-8])
            ct = re.findall(r"compile Time: (.+) count:", res[-7])
            at = re.findall(r"aot Time: (.+) count:", res[-5])
            jt = re.findall(r"jit Time: (.+) count:", res[-3])
            # overhead = -999999
            # if float(at[0])-float(jt[0]) > 0:
            #     overhead = float(ct[0]) / (float(at[0])-float(jt[0]))
            # print(llvm_ct)
            data['llvmcompile time'].append(float(llvm_ct[0]))
            data['compile time'].append(float(ct[0]))
            data['aot time'].append(float(at[0]))
            data['jit time'].append(float(jt[0]))
            gflops = re.findall(r"aot GFLOPS: (.+) count:", res[-4])
            data['aot'].append(float(gflops[0]))
            gflops = re.findall(r"jit GFLOPS: (.+) count:", res[-2])
            data['jit'].append(float(gflops[0]))
            i = i + 1
    return data

def cache(data, path):
    # print(data)
    assert(len(data[0])==len(data[1]))
    num = len(data[0])
    with open(path, "w") as f:
        for i in range(0, num):
            if data[1][i]>=0:
                f.write(data[0][i]+" "+str(data[1][i])+"\n")

def cache_dynvec(data, root_path):
    cache((data['name'],data['jit']), root_path+"/log_dynvec.dat")
    cache((data['name'],data['aot']), root_path+"/log_icc.dat")
    cache((data['name'],data['jit time']), root_path+"/dynvec_time.dat")
    cache((data['name'],data['aot time']), root_path+"/icc_time.dat")
    cache((data['name'],data['llvmcompile time']), root_path+"/llvmcompile_time.dat")
    cache((data['name'],data['compile time']), root_path+"/compile_time.dat")

if __name__ == "__main__":
    log_root = argv[1]
    print("Extracting DynVec Data ...")
    data_dynvec = extract_dynvec(log_root+"/log_spmv_dynvec_sve")
    cache_dynvec(data_dynvec, log_root)