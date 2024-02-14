#!/bin/bash

if [ $# -lt 4 ]
then
	echo "Args: [generator program path] [data type] [arrangement] [out suffix]"
	exit -1
fi

path_generator=$1
type=$2
arrange=$3
suffix=$4

declare -rA amounts=(
	#["1e3"]=1024
	#["2e3"]=2000
	#["5e3"]=5000
	#["1e4"]=10000
	#["2e4"]=20000
	#["5e4"]=50000
	#["1e5"]=100000
	#["2e5"]=200000
	#["5e5"]=500000
	["1e6"]=1000000
	["2e6"]=2000000
	["5e6"]=5000000
	["1e7"]=10000000
	["2e7"]=20000000
	["5e7"]=50000000
	["1e8"]=100000000
	["2e8"]=200000000
	["5e8"]=500000000
	["1e9"]=1000000000
	["2e9"]=2000000000
)

for num_str in "${!amounts[@]}"
do
	num="${amounts[$num_str]}"
	
	file_name="d_${num_str}_${type}_${suffix}.bin"
	
	echo "${file_name}"
	$1 $num $type $arrange "-b" $file_name
done
