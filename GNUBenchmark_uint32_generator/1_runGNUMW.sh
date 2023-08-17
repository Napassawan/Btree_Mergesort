#!/bin/bash
#echo 0 > /proc/sys/kernel/nmi_watchdog
#echo "Complie"
#gcc -o $1 $1.c -O2 #-fopenmp
#g++ -o MSTSort $1.cpp -O2 -fopenmp

#export OMP_PROC_BIND=true
#export OMP_PLACES=cores
#export OMP_PLACES=threads
#export OMP_PROC_BIND=close
#export OMP_PROC_BIND=spread
shut=0
max=10

max=$1
chain=$2

folder='MW'
folder='MW_'$max'x'
Ptxt1='_Random_'
Ptxt2='L3_O2.txt'
Stxt='_Random_O2.txt'
mkdir -p $folder
#rename $'\r' '' *
#echo "make $folder folder completed!!"
#echo "done"
#echo "start process"
#echo "running parallel"

	filename_p=$folder"/"$Ptxt1$Ptxt2
	echo "BEGIN_EXP">>$filename_p
	#for n in 10000000 20000000 50000000 100000000 200000000 500000000 1000000000 2000000000
	for n in 200000000 500000000 1000000000 2000000000
	do
		
	       	#echo "running Cutoff at $u"
		#echo "----------- Start run Cutoff at $u   ---------">>$filename_
							
		for i in `seq 1 $max`
		do
			echo "BEGIN_ITER">>$filename_p
			echo "PARAM Size : $n">>$filename_p
			echo "PARAM Multithread : 1">>$filename_p
			echo "PARAM Multithread2 : 1">>$filename_p

			./mainMW $n >>$filename_p 2>&1
			sleep 1s
			#perf stat -r 5 -e cpu-cycles,instructions,cache-references,cache-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,dTLB-loads,dTLB-load-misses,dTLB-stores,dTLB-store-misses,iTLB-loads,iTLB-load-misses,branch-loads,branch-load-misses,page-faults,context-switches,cpu-migrations ./mainTBB $n'_uint32.dat' $b $u $s $m $r>>$filename_p 2>&1
			echo "END_ITER">>$filename_p
		done		
		zip $folder.zip $folder/*
	done
echo "END_EXP">>$filename_p

zip $folder.zip $folder/*

if [ $shut -eq 1 ]
then
sleep 10s
shutdown -h now
fi

