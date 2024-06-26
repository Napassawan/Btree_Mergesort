#include <fstream>
#include <string>
#include <memory>

#include <execution>
#include <algorithm>

#include <malloc.h>
#include <parallel/algorithm>
#include <omp.h>

#include "../common/util.hpp"
#include "../common/reader.hpp"

#include "timer.hpp"
#include "btree_sort.hpp"

#ifdef WINDOWS
	#pragma message("Compile mode: Windows")
#else
	#pragma message("Compile mode: UNIX")
#endif

using std::string;
using std::vector;

using std::unique_ptr;

// ------------------------------------------------------------------------------

void Work(DataType type, SortType sort, const FileReader& file);

// Static timer so I don't have to throw it across 73 functions
PerformanceTimer timer;

size_t runCount = 1;

// ------------------------------------------------------------------------------

void PrintHelp()
{
	printf("Arguments: DataType Mode Input [option...]\n");
	printf("    DataType can be any of:\n");
	printf("        i32, u32, i64, u64, f64\n");
	printf("    Mode can be:\n");
	printf("        mw          Multiway Mergesort\n");
	printf("        bq          Balanced Quicksort\n");
	printf("        bt          B-Tree Sort\n");
	printf("    Input can be:\n");
	printf("        -b FILE     Read input as binary file\n");
	printf("        -t FILE     Read input as text file\n");
	printf("    Option can be:\n");
	printf("        -m [cv]     Benchmark result options\n");
	printf("            c           Compact result\n");
	printf("            v           Verbose result\n");
	printf("        -n [num]    Repeat count\n");
}
int main(int argc, char** argv)
{
	if (argc < 4) {
		PrintHelp();
		return 0;
	}

	bool bReport = false;
	bool bCompact = false;
	bool bVerbose = false;

	FileReader input;

	{
		// Start parsing after Mode arg
		OptParse optParse(argc - 2, argv + 2);

		if (optParse.OptionExists("-m")) {
			if (auto opt = optParse.GetOptionParam("-m")) {
				bCompact = opt->get().find('c') != std::string::npos;
				bVerbose = opt->get().find('v') != std::string::npos;
			}
			bReport = true;
		}

		if (optParse.OptionExists("-b")) {
			if (auto opt = optParse.GetOptionParam("-b")) {
				input = FileReader(*opt, true);
			}
			else {
				printf("-b: File name is required\n");
				return -1;
			}
		}
		else if (optParse.OptionExists("-t")) {
			if (auto opt = optParse.GetOptionParam("-t")) {
				input = FileReader(*opt, false);
			}
			else {
				printf("-t: File name is required\n");
				return -1;
			}
		}

		if (optParse.OptionExists("-n")) {
			if (auto opt = optParse.GetOptionParam("-n")) {
				runCount = strtoul(opt->get().c_str(), nullptr, 10);
			}
			else {
				printf("-n: Amount is required\n");
				return -1;
			}
		}
	}

	DataType typeDataParse = GetDataTypeFromString(argv[1]);
	SortType typeSort = GetSortTypeFromString(argv[2]);

	if (input.path.empty() || typeDataParse == DataType::Invalid || typeSort == SortType::Invalid) {
		PrintHelp();
		return 0;
	}

	try {
		Work(typeDataParse, typeSort, input);

		if (bReport)
			timer.Report(std::cout, bCompact, bVerbose);
	}
	catch (const string& e) {
		printf("Fatal error-> %s", e.c_str());
	}

	printf("\n");
	return 0;
}

// ------------------------------------------------------------------------------

template<typename T> void WorkGeneric(SortType sort, const FileReader& file);
template<typename T> void PerformSort(SortType sort, vector<T>& res);
template<typename T> void VerifySorted(vector<T>& data);

void Work(DataType type, SortType sort, const FileReader& file)
{
	switch (type) {
	case DataType::i32: WorkGeneric<int32_t>(sort, file);	break;
	case DataType::u32: WorkGeneric<uint32_t>(sort, file);	break;
	case DataType::i64: WorkGeneric<int64_t>(sort, file);	break;
	case DataType::u64: WorkGeneric<uint64_t>(sort, file);	break;
	case DataType::f64: WorkGeneric<double>(sort, file);	break;
	default: break;
	}
}
template<typename T> void WorkGeneric(SortType sort, const FileReader& file)
{
	for (size_t i = 0; i < runCount; ++i) {
		vector<T> data = file.ReadData<T>();
		
		if (i == 0) {
			printf("Read %zu data from file (%zu bytes)\n", 
				data.size(), data.size() * sizeof(T));
			printf("Repeat: %zu\n", runCount);
		}

		timer.Start();
		
		PerformSort(sort, data);
		
		auto stat = timer.Stop();
		
		timer.AddDataPoint(stat);

		if (i == 0) {
			VerifySorted(data);
		}
	}
	std::cout << "\n";
}

template<typename T> void PerformSort(SortType sort, vector<T>& res)
{
	switch (sort) {
	case SortType::MultiwayMerge:
		__gnu_parallel::sort(res.begin(), res.end(),
			__gnu_parallel::multiway_mergesort_tag());
		break;
	case SortType::BalancedQuick:
		__gnu_parallel::sort(res.begin(), res.end(),
			__gnu_parallel::balanced_quicksort_tag());
		break;
	case SortType::BTreeMerge: {
		btreesort::BTreeSort btreesort(
			res.begin(), res.end(), std::less<T>());
		btreesort.Sort();
		
		break;
	}
	default: break;
	}
}

template<typename T> void VerifySorted(vector<T>& data)
{
	bool sorted = std::is_sorted(std::execution::par, 
		data.cbegin(), data.cend(), std::less<T>());
	if (sorted) {
		printf("Sort verified\n");
	}
	else {
		printf("Sort failed, some elements out of order\n");
		
		// TODO: Maybe print failed elements
	}
}
