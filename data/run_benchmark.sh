#!/bin/bash

if [ $# -lt 5 ]
then
	echo "Args: [benchmark program path] [file match] [data type] [mode] [report dest]"
	exit -1
fi

path_bench=$1
file_match=$2
data_type=$3
mode=$4
report_dest=$5

if ! [ -f $path_bench ]; then
	echo "program doesn't exist"
	exit -1
fi
if ! [ -d $report_dest ]; then
	echo "dest directory doesn't exist"
	exit -1
fi

declare -ra run_count_map=(
	"4,20000"
	"5,5000"
	"6,500"
	"7,100"
	"8,100"
	"9999,50"
)

for data_file in $file_match; do
	report_file="${report_dest}/${mode}_${data_file%.*}.txt"
	count_mag=$(echo $data_file | cut -d "_" -f 2 | cut -d "e" -f 2)
	
	for e_bound in "${run_count_map[@]}"; do
		IFS=','; read -a rc <<< "$e_bound"
		bound=${rc[0]}; count=${rc[1]};
		
		if [[ $count_mag -le $bound ]]; then
			run_count=$count
			break
		fi
	done
	
	echo "${data_file} > ${report_file} (repetition=${run_count})"
	$path_bench $data_type $mode -b $data_file -m cv -n $run_count > $report_file
done
