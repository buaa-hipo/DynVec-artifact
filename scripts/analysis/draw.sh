#!/usr/bin/env bash

###
### draw.sh -- plot the performance data
### Usage:
### 	draw.sh <arg1> <arg2>
###
### Options:
### 	<arg1>	The path of the raw data log
### 	<arg2>	The path of the extracted log
###	-h	Show this message.

help() {
	sed -rn 's/^### ?//;T;p;' "$0"
}

if [[ $1 = "--help" ]] || [[ $1 = "-h" ]]
then
	help
	exit 1
fi
python3 extract_arm_sve_data.py $1
python3 performance.py $2
