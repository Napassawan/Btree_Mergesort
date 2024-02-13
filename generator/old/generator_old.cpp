#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <alloca.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <stack>
#include <deque>
#include <algorithm>

#include <iostream>
#include <omp.h>
#include <parallel/algorithm>
#include <cfloat>

#define SORT_SWAP(x,y) {DataType __SORT_SWAP_t = (x); (x) = (y); (y) = __SORT_SWAP_t;}

#define N 10000

#define INDEX uint64_t

#define UINT32TYPE 1
#define UINT64TYPE 0
#define DOUBLETYPE 0

#if UINT32TYPE
#define DataType uint32_t
#define DataMax UINT32_MAX
#elif UINT64TYPE
#define DataType uint64_t
#define DataMax UINT64_MAX
#elif DOUBLETYPE
#define DataType double
#define DataMax DBL_MAX
#endif

#define UNIQUENUM 100
#define RANDSWAPPERCENT 99

#define REVERSED 0
#define FEWUNIQUE 0
//NEARLY SORTED AND RANDOM DIST. -> SET REVERSED AND FEW UNIQUE TO 0 THEN DEFINE RANDSWAPPERCENT

void newinitData(DataType *arr, INDEX num){
	srand(time(NULL));
	
	
#if DOUBLETYPE
	for(INDEX i=0;i<num;i++){
		arr[i] = rand();
	}
#else 
	INDEX max = DataMax;
	INDEX inc = DataMax/num;
	for(INDEX i=0;i<num;i++){
		arr[i] = i*inc;
	}
#endif

#if REVERSED
	__gnu_parallel::sort(&arr[0],&arr[num-1],std::greater<DataType>(),__gnu_parallel::balanced_quicksort_tag());
#else 
	#if FEWUNIQUE
		DataType unique[UNIQUENUM];
		
		#if DOUBLETYPE
			for(INDEX i=0;i<UNIQUENUM;i++){
				unique[i] = rand();
			}
			for(INDEX i=0;i<num;i++){
        		arr[i] = unique[rand()%UNIQUENUM];
   	 		}
		#else 
			for(INDEX i=0;i<UNIQUENUM;i++){
				unique[i] = rand()%max;
			}
			for(INDEX i=0;i<num;i++){
        		arr[i] = unique[rand()%UNIQUENUM];
   	 		}
		#endif
	#else //NEARLYSORTED OR RANDOM
		#if DOUBLETYPE
			//DOUBLE TYPE SHOULD BE SORTED BEFORE
			__gnu_parallel::sort(&arr[0],&arr[num-1],__gnu_parallel::balanced_quicksort_tag());
		#else
			//UINT TYPES SHOULD NOT BE SORTED 
		#endif
			//RANDOM SWAP TO GENERATE RANDOM OR NEARLYSORTED
			for(INDEX i=0;i<num*RANDSWAPPERCENT/100;i++){
				SORT_SWAP(arr[rand()%num], arr[rand()%num]);
			}
	#endif
#endif
	
int main(){
	DataType* arr = (DataType*)malloc(sizeof(DataType)*N);
	//initData(arr,N);
	newinitData(arr, N);
	int i;
	for(i=0;i<N;i++){
		std::cout<<arr[i]<<" ";
	}
	return 0;
}