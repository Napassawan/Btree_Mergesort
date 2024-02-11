#!/bin/bash

if [ -z "$1" ]
then
	echo "Must specify generator path"
	exit -1
fi

path_generator=$1

declare -rA amounts=(
	#["1e3"]=1024
	#["2e3"]=2000
	#["5e3"]=5000
	["1e4"]=10000
	["2e4"]=20000
	["5e4"]=50000
	["1e5"]=100000
	["2e5"]=200000
	["5e5"]=500000
	["1e6"]=1000000
	["2e6"]=2000000
	["5e6"]=5000000
	["1e7"]=10000000
	#["2e7"]=20000000
	#["5e7"]=50000000
	#["1e8"]=100000000
	#["2e8"]=200000000
	#["5e8"]=500000000
	#["1e9"]=1000000000
)

declare -ra types=(
	"i32"
	"i64"
	#"u32"
	#"u64"
	"f64"
)

declare -rA arranges=(
	["ran"]="random"
	["rev"]="reversed"
	["fu"]="fewunique"
	["ns"]="nsorted"
)

for num_str in "${!amounts[@]}"
do
	for ty in "${types[@]}"
	do
		for arr in "${!arranges[@]}"
		do
			num="${amounts[$num_str]}"
			arr_arg="${arranges[$arr]}"
			
			file_name="d_${num_str}_${ty}_${arr}.bin"
			
			echo "${file_name}"
			$1 $num $ty $arr_arg "-b" $file_name
		done
	done
done
