#include <fstream>
#include <string>
#include <memory>

#include <execution>
#include <algorithm>

#include <parallel/algorithm>
#include <omp.h>

#include "../Common/mygetopt.hpp"

#include "timer.hpp"

#ifdef WINDOWS
	#pragma message("Compile mode: Windows")
#else
	#pragma message("Compile mode: UNIX")
#endif

using std::string;
using std::vector;

using std::unique_ptr;

// ------------------------------------------------------------------------------

enum class DataType {
	i32,
	u32,
	i64,
	u64,
	f64,
	Invalid,
};
DataType GetDataTypeFromString(char* name);

enum class SortType {
	MultiwayMerge,
	BalancedQuick,
	BTreeMerge,
	Invalid,
};
SortType GetSortTypeFromString(char* type);

struct InFile {
	string path;
	bool binary;
};

// ------------------------------------------------------------------------------

void Work(DataType type, SortType sort, const InFile& file);

// Static timer so I don't have to throw it across 73 functions
PerformanceTimer timer;

// ------------------------------------------------------------------------------

void PrintHelp() {
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
}
int main(int argc, char** argv) {
	if (argc < 4) {
		PrintHelp();
		return 0;
	}
	
	bool bCompact = false;
	bool bVerbose = false;
	
	InFile input;
	
	{
		// Start parsing after Mode arg
		OptParse optParse(argc - 2, argv + 2);
		
		if (optParse.OptionExists("-m")) {
			if (auto opt = optParse.GetOptionParam("-m")) {
				bCompact = strchr(opt->get().c_str(), 'c');
				bVerbose = strchr(opt->get().c_str(), 'v');
			}
		}
		
		if (optParse.OptionExists("-b")) {
			if (auto opt = optParse.GetOptionParam("-b")) {
				input.binary = true;
				input.path = *opt;
			}
			else {
				printf("-b: File name is required\n");
				return -1;
			}
		}
		else if (optParse.OptionExists("-t")) {
			if (auto opt = optParse.GetOptionParam("-t")) {
				input.binary = false;
				input.path = *opt;
			}
			else {
				printf("-t: File name is required\n");
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
		//timer.Start();
		
		Work(typeDataParse, typeSort, input);
		
		//timer.Stop();
		timer.Report(std::cout, bCompact, bVerbose);
	}
	catch (const string& e) {
		printf("Fatal error-> %s", e.c_str());
	}
	
	//printf("Done");
	return 0;
}

// ------------------------------------------------------------------------------

#ifndef WINDOWS
	#define strcmpi strcasecmp
#endif

DataType GetDataTypeFromString(char* name) {
#define CHECK(_chk, _type) if (strcmpi(name, _chk) == 0) return _type
	
	CHECK("i32", DataType::i32);
	else CHECK("u32", DataType::u32);
	else CHECK("i64", DataType::i64);
	else CHECK("u64", DataType::u64);
	else CHECK("f64", DataType::f64);
	else CHECK("double", DataType::f64);
	
	return DataType::Invalid;

#undef CHECK
}

SortType GetSortTypeFromString(char* type) {
#define CHECK(_chk, _type) if (strcmpi(type, _chk) == 0) return _type
	
	CHECK("mw", SortType::MultiwayMerge);
	else CHECK("bq", SortType::BalancedQuick);
	else CHECK("bt", SortType::BTreeMerge);
	
	return SortType::Invalid;

#undef CHECK
}

// ------------------------------------------------------------------------------

template<typename T> void WorkGeneric(SortType sort, const InFile& file);
template<typename T> void ReadDataFromFile(vector<T>& res, const InFile& file);
template<typename T> void PerformSort(SortType sort, vector<T>& res);
template<typename T> void VerifySorted(vector<T>& data);

void Work(DataType type, SortType sort, const InFile& file) {
	switch (type) {
	case DataType::i32:
		WorkGeneric<int32_t>(sort, file);
		break;
	case DataType::u32:
		WorkGeneric<uint32_t>(sort, file);
		break;
	case DataType::i64:
		WorkGeneric<int64_t>(sort, file);
		break;
	case DataType::u64:
		WorkGeneric<uint64_t>(sort, file);
		break;
	case DataType::f64:
		WorkGeneric<double_t>(sort, file);
		break;
	default: break;
	}
}
template<typename T> void WorkGeneric(SortType sort, const InFile& file) {
	vector<T> data;
	
	ReadDataFromFile<T>(data, file);
	
	PerformSort(sort, data);
	
	/* for (const T& i : data) {
		std::cout << i << " ";
	} */
	
	VerifySorted(data);
	
	printf("\n");
}

template<typename T> void ReadDataFromFile(vector<T>& res, const InFile& fileData) {
	std::ifstream file(fileData.path, 
		fileData.binary ? std::ios::binary : (std::ios::openmode)0);
	if (!file.is_open())
		throw string("Failed to open file for reading");
	
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	
	if (fileData.binary) {
		if (fileSize % sizeof(T) != 0)
			throw string("Wrong file size for data type");
		
		size_t dataCount = fileSize / sizeof(T);
		res.resize(dataCount);
		
		// Buffered read into res
		{
			constexpr size_t MAX_PER_IT = 4096;
			
			size_t pos = 0;
			while (true) {
				file.read((char*)&res[pos], MAX_PER_IT * sizeof(T));
				
				size_t read = file.gcount();
				if (read == 0)		// EOF reached
					break;
				
				pos += read;
			}
		}
	}
	else {
		// Use file exceptions instead of putting checks inside the read loop
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		
		try {
			T value;
			while (file >> value) {
				res.push_back(value);
			}
		}
		catch (const std::ifstream::failure& e) {
			//throw (string("File read error: ") + e.what());
		}
	}
	
	printf("Read %zu data from file (%zu bytes)\n", 
		res.size(), res.size() * sizeof(T));
	
	file.close();
}

template<typename T> void PerformSort(SortType sort, vector<T>& res) {
	timer.Start();
	
	switch (sort) {
	case SortType::MultiwayMerge:
		__gnu_parallel::sort(res.begin(), res.end(), __gnu_parallel::multiway_mergesort_tag());
		break;
	case SortType::BalancedQuick:
		__gnu_parallel::sort(res.begin(), res.end(), __gnu_parallel::balanced_quicksort_tag());
		break;
	case SortType::BTreeMerge:
		throw string("Not implemented");
		break;
	default: break;
	}
	
	timer.Stop();
}

template<typename T> void VerifySorted(vector<T>& data) {
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
