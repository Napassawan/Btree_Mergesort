#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <cfloat>

#include <iostream>
#include <omp.h>
#include <parallel/algorithm>

#define BQ 1

#define VERIFYDATA 0
#define NESTED 1

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

//NEARLY SORTED -> SET REVERSED AND FEW UNIQUE TO 0 and NSORTED to 1 THEN DEFINE RANDFACTOR
//RANDOM -> SET REVERSED, FEWUNIQUE, NSORTED TO 0 THEN DEFINE RANDSWAPPERCENT

#define REVERSED 0
#define FEWUNIQUE 0
#define NSORTED 0

#define UNIQUEPERCENT 1
#define RANDFACTOR 4
#define RANDSWAPPERCENT 100


#define NESTED 1

#define UTIL 1

#define MAX_CPU 32

#define SORT_SWAP(x,y) {DataType __SORT_SWAP_t = (x); (x) = (y); (y) = __SORT_SWAP_t;}


#define SWAP(a, b, size)						      \
  do									      \
    {									      \
      size_t __size = (size);						      \
      char *__a = (a), *__b = (b);					      \
      do								      \
	{								      \
	  char __tmp = *__a;						      \
	  *__a++ = *__b;						      \
	  *__b++ = __tmp;						      \
	} while (--__size > 0);						      \
    } while (0)
	
typedef struct {
    char *cpu_core;
	unsigned int user;
	unsigned int nice;
	unsigned int syst;
	unsigned int idle;
} cpu_info_t;

struct timeval startwtime, endwtime;
double seq_time;

INDEX nmem;
DataType thread;
DataType par_partition_cutoff;
DataType BLOCK_SIZE;
DataType CUTOFFBLOCK;
DataType CACHETOTALSIZE;
DataType lthread;
DataType globalthread;


using namespace std;

void showData(DataType *arr,INDEX num){
#if DOUBLETYPE
	for(INDEX i=0;i<num;i++){
		printf("%f ",arr[i]);
	}
#else 
	for(INDEX i=0;i<num;i++){
		printf("%llu ",arr[i]);
	}
#endif

}

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
		//DataType unique[UNIQUENUM];
		INDEX uniquenum = UNIQUEPERCENT*num/100;
		DataType* unique = (DataType*)malloc(sizeof(DataType)*uniquenum);
		
		#if DOUBLETYPE
			for(INDEX i=0;i<uniquenum;i++){
				unique[i] = rand();
			}
			for(INDEX i=0;i<num;i++){
        		arr[i] = unique[rand()%uniquenum];
   	 		}
		#else 
			for(INDEX i=0;i<uniquenum;i++){
				unique[i] = rand()%max;
			}
			for(INDEX i=0;i<num;i++){
        		arr[i] = unique[rand()%uniquenum];
   	 		}
		#endif
		free(unique);
	#else //NEARLYSORTED OR RANDOM
		#if DOUBLETYPE
			//DOUBLE TYPE SHOULD BE SORTED BEFORE
			__gnu_parallel::sort(&arr[0],&arr[num-1],__gnu_parallel::balanced_quicksort_tag());
		#else
			//UINT TYPES SHOULD NOT BE SORTED 
		#endif
			#if NSORTED
			for(INDEX i=0;i<(int)sqrt(num)*RANDFACTOR;i++){
				SORT_SWAP(arr[rand()%num], arr[rand()%num]);
			}
			#else
			for(INDEX i=0;i<num*RANDSWAPPERCENT/100;i++){
				SORT_SWAP(arr[rand()%num], arr[rand()%num]);
			}
			#endif
	#endif
#endif
}

void verifydata(DataType *arr,INDEX num){
    INDEX verify = 0;
    for(INDEX i=0;i<num-1;i++){
    	if(arr[i]==0){
    		printf("%u at %u\n",arr[i],i);
    	}
        if(arr[i]>arr[i+1]){
            printf("failed at %u>%u at %d\n",arr[i],arr[i+1],i);
            verify++;
        }
    }
    if (verify) {
        printf("\nFailed %d\n",verify);
    }
    else{
        printf("\nPassed\n");
    }
}

void verifylower(DataType *arr,INDEX left,INDEX right,INDEX pivot){
	INDEX verify = 0;
    for (INDEX i=left; i<=right; i++) {
        if (arr[i]>arr[pivot]) {
            printf("%d at %d > %d\n",arr[i],i,arr[pivot]);
            verify++;
        }
    }
    if (verify) {
        printf("Lower Partition Failed %d\n",verify);
    }
    else{
        printf("Lower Partition Passed\n");
    }
}

void verifyhigher(DataType *arr,INDEX left, INDEX right, INDEX pivot){
	INDEX verify = 0;
    for (INDEX i=left; i<=right; i++) {
        if (arr[i]<=arr[pivot]) {
            printf("%d at %d <= %d\n",arr[i],i,arr[pivot]);
            verify++;
        }
    }
    if (verify) {
        printf("Higher Partition Failed %d\n",verify);
    }
    else{
        printf("Higher Partition Passed\n");
    }
}

void verifypartition(DataType *arr,INDEX left,INDEX right,INDEX mid,INDEX pivot){
    INDEX verify = 0;
    for (INDEX i=left; i<mid-1; i++) {
        if (arr[i]>arr[pivot]) {
            printf("%d at %d > %d\n",arr[i],i,arr[pivot]);
            verify++;
        }
    }
    for (INDEX i=mid+1; i<right; i++) {
        if (arr[i]<=arr[pivot]) {
            printf("%d at %d <= %d\n",arr[i],i,arr[pivot]);
            verify++;
        }
    }
    if (verify) {
        printf("Partition Failed %d\n",verify);
    }
    else{
        printf("Partition Passed\n");
    }
}


int measure_cpu_uti(cpu_info_t *list, int NCores) 
{
    FILE *cpu_fptr  = fopen("/proc/stat", "rb");
	char *buf1 = (char*)malloc(256);
	char buf[256];
	int cpu_count = 0;
	
	while ( cpu_count < NCores + 1) {
      char *temp =fgets( buf, sizeof(buf), cpu_fptr );
		
	  if ( strncmp( buf, "cpu", 3 ) == 0 ) 
		{
		  strncpy(buf1, buf, sizeof(buf));
#if IOWAITUTIL

#else
		  char *cpu_core = (char*)malloc(sizeof(char)*4);
#endif
		  sscanf(buf1,"%s %d %d %d %d",cpu_core, &list[cpu_count].user,&list[cpu_count].nice,&list[cpu_count].syst,&list[cpu_count].idle);

		  list[cpu_count].cpu_core = cpu_core;
          cpu_count++;
		}
	}
	
	// print out
	int i;
  //  for (i = 0; i <= NCores; ++i)
      //  printf("%s %d %d %d %d\n", list[i].cpu_core, list[i].user,list[i].nice,list[i].syst,list[i].idle); 
	if(buf1)	
		free(buf1);
	fclose(cpu_fptr);  	
	
    return EXIT_SUCCESS;
}



void cal_cpu_uti(cpu_info_t *list1, cpu_info_t *list2, int NCores)
{
     int i;
	 DataType temp[4];
	 float result[4];
	 //printf("\npercentage of cpu utilizaton\n");
     for (i = 0; i <= NCores; ++i)
	 {
	   float sum = 0.0; temp[0]=temp[1]=temp[2]=temp[3] = 0;
	   temp[0] = list2[i].user - list1[i].user;
	   temp[1] = list2[i].nice - list1[i].nice;
	   temp[2] = list2[i].syst - list1[i].syst;
	   temp[3] = list2[i].idle - list1[i].idle;
	   //int sum = (list1[i].user - list2[i].user) + (list1[i].nice - list2[i].nice) + (list1[i].syst - list2[i].syst) + (list1[i].idle - list2[i].idle);
	   sum = temp[0] + temp[1] + temp[2] + temp[3];
	   
	   result[0] = temp[0]/sum;
	   result[1] = temp[1]/sum;
	   result[2] = temp[2]/sum;
	   result[3] = temp[3]/sum;
	   
	   printf("%s %10.2f %10.2f %10.2f %10.2f\n", list1[i].cpu_core, result[0]*100,result[1]*100,result[2]*100,result[3]*100); 
	 }

}


void swap(DataType *x,DataType *y)
{
    DataType temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

void Sort(DataType *arr, const size_t size) {
	INDEX left = 0;
	INDEX right = size-1;
	char *lo = (char *)(arr);
	char *hi = &lo[sizeof(DataType) * (size - 1)];	
    if (size == 0) {
        return;
    }

#if BQ
	
	//printf("BQSORT\n");
    	__gnu_parallel::sort(&arr[left],&arr[right+1],__gnu_parallel::balanced_quicksort_tag());
	//__gnu_parallel::sort(elements.begin(), elements.end(), __gnu_parallel::quicksort_tag());
	//__gnu_parallel::sort(elements.begin(), elements.end(), __gnu_parallel::multiway_mergesort_tag());
#else 
	//printf("MWSORT\n");
	//__gnu_parallel::sort(&arr[left],&arr[right+1]);
    	__gnu_parallel::sort(&arr[left],&arr[right+1],__gnu_parallel::multiway_mergesort_tag());
#endif
}

int main(int argc, char **argv) {

    FILE *fp, *fp_w;
    int NCores;
    int cpu_count = 0;
    
    INDEX SIZE = atol(argv[1]);

	//printf("size = %lld\n",SIZE);

	DataType* arr = (DataType*)malloc(sizeof(DataType)*SIZE);

    newinitData(arr,SIZE);
    //printf("Init data Completed\n");

#ifdef __APPLE__
	
#else
#if UTIL
    cpu_info_t *list_first = (cpu_info_t*)malloc(sizeof(cpu_info_t) * MAX_CPU);
	
	cpu_info_t *list_second = (cpu_info_t *)malloc(sizeof(cpu_info_t) * MAX_CPU);
	
	fp = popen("cat /proc/cpuinfo | grep processor | wc -l","r");
    char temp = fscanf(fp, "%d", &NCores);
	
	fclose(fp);
	
#else
	
#endif
#endif	
	
	
	//printf("Sorting\n");
	
    double start_time = omp_get_wtime();
	
#ifdef __APPLE__

#else
#if UTIL
	measure_cpu_uti(list_first,NCores);
#endif
#endif
	omp_set_nested(1);
	Sort(arr, SIZE);

	double time = omp_get_wtime() - start_time;
    
    //printf("Sorting Finished\n");
    
	printf("\nTIME %f\n",time);
	
#ifdef __APPLE__

#else
#if UTIL
	measure_cpu_uti(list_second,NCores);

	// calculate cpu uti
	cal_cpu_uti(list_first,list_second,NCores);
#endif
#endif
	
#if VERIFYDATA
	printf("Verifying\n");
    verifydata(arr,SIZE);
#endif
    
	free(arr);

	return 0;
}

