#! /usr/bin/python3
import os
import re
import json
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

def extract_cvr(path_log):
    begin = "-----"
    log_file = open(path_log)
    text = log_file.read()
    results = re.findall("Warning.*|correct|Sorry.*", text);
    names = re.findall(r"File Path:[^\n]*mx_data_less_than_1G/(.+).mtx", text)
    preprocessing_times = re.findall(r"Pre-processing\(CSR->CVR\)   Time of CVR   is ([-0-9.e]+) seconds", text)
    spmv_times = re.findall(r"Execution Time of CVR    is ([-0-9.e]+) seconds", text)
    throughputs = re.findall(r"Throughput of CVR    is ([-0-9.e]+) GFlops", text) 
    data = []
    timeptr = 0
    for i in range(len(names)):
        if (i == len(names) - 1) and (len(names) != len(results)):
            print("len(names) != len(results), is the test going on?")
            break
        item = {}
        invalid = results[i].startswith("Sorry")
        if invalid:
            continue
        item['status'] = results[i]
        item['name'] = names[i]
        item['preprocessing_time'] = preprocessing_times[timeptr]
        item['spmv_time'] = spmv_times[timeptr]
        item['throughput'] = throughputs[timeptr]
        timeptr += 1
        data.append(item)
    # print(json.dumps(data, indent=2))
    # print(len(names), len(preprocessing_times), len(spmv_times), len(throughputs), len(results), len(data))
    return data

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

def cache_cvr(data, root_path):
    names = list(map(lambda x: x['name'], data))
    preprocessing_times = list(map(lambda x: float(x['preprocessing_time']), data))
    perfs = list(map(lambda x: float(x['throughput']), data))
    cache((names,perfs), root_path+"/cvr.dat")
    cache((names,preprocessing_times), root_path+"/cvr_overhead.dat")

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
    print("Extracting CVR Data ...")
    data_cvr = extract_cvr(log_root+"/log_spmv_cvr")
    cache_cvr(data_cvr, log_root)