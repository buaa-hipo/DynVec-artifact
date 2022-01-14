#! /usr/bin/python3
import os
import re
from sys import argv

def extract_dynvec(path_log):
    data = {
        'name' : [],
        'jit'  : [],
        'aot'  : [],
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
            ct = re.findall(r"compile Time: (.+) count:", res[4])
            at = re.findall(r"aot Time: (.+) count:", res[len(res)-5])
            jt = re.findall(r"jit Time: (.+) count:", res[len(res)-3])
            overhead = -999999
            if float(at[0])-float(jt[0]) > 0:
                overhead = float(ct[0]) / (float(at[0])-float(jt[0]))
            data['overhead'].append(overhead)
            gflops = re.findall(r"aot GFLOPS: (.+) count:", res[len(res)-4])
            data['aot'].append(float(gflops[0]))
            gflops = re.findall(r"jit GFLOPS: (.+) count:", res[len(res)-2])
            data['jit'].append(float(gflops[0]))
            i = i + 1
    return data

def extract_csr5(path_log):
    name = []
    perf = []
    begin0 = "------------------------------------------------------\n"
    begin1 = "PRECISION = 64-bit Double Precision\n"
    begin2 = "------------------------------------------------------\n"
    log_file = open(path_log)
    lines = log_file.readlines()
    result = []
    size_lines = len(lines)
    i = 4
    new_result = []
    while i < size_lines:
        while i < size_lines : 
            if( i + 3 < size_lines and lines[i] == begin0 
                and lines[i+1] == begin1 
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
            name.append(file_name[0])
            perf.append(float(gflops[0]))
            i = i + 1
    return name, perf

def extract_mkl(path_log):
    name = []
    perf = []
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
            file_name = re.findall(r"mx_data_less_than_1G/(.+).mtx", res[0])
            
            gflops = re.findall(r"GFLOPS: (.+) count:", res[3])
            name.append(file_name[0])
            perf.append(float(gflops[0]))
            i = i + 1
    return name, perf

def cache(data, path):
    assert(len(data[0])==len(data[1]))
    num = len(data[0])
    with open(path, "w") as f:
        for i in range(0, num):
            if data[1][i]>=0:
                f.write(data[0][i]+" "+str(data[1][i])+"\n")

def cache_dynvec(data, root_path):
    cache((data['name'],data['jit']), root_path+"/dynvec.dat")
    cache((data['name'],data['aot']), root_path+"/icc.dat")
    cache((data['name'],data['overhead']), root_path+"/dynvec_overhead.dat")

if __name__ == "__main__":
    log_root = argv[1]
    print("Extracting DynVec Data ...")
    data_dynvec = extract_dynvec(log_root+"/log_spmv_dynvec")
    cache_dynvec(data_dynvec, log_root)
    print("Extracting CSR5 Data ...")
    data_csr5 = extract_csr5(log_root+"/log_spmv_csr5")
    cache(data_csr5, log_root+"/csr5.dat")
    print("Extracting MKL Data ...")
    data_mkl = extract_mkl(log_root+"/log_spmv_mkl")
    cache(data_mkl, log_root+"/mkl.dat")