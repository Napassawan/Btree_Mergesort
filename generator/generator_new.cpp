#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cfloat>
#include <ctime>

#include <vector>

#include <random>
#include <functional>
#include <execution>
#include <algorithm>

#include <string.h>
#include <omp.h>

#include "../common/util.hpp"

using std::string;
using std::vector;

using std::unique_ptr;

// ------------------------------------------------------------------------------

constexpr const float Opt_FewUnique_UniquePercentage = 0.01; // 1%

constexpr const float Opt_NSorted_SwapPercentage = 0.05; // 5%

// ------------------------------------------------------------------------------

void GenerateDataFromType(size_t count, DataType type, DataArrangeType arrangement);
template<typename T> void GenerateDataFromArrangement(size_t count, DataArrangeType arrangement);

template<typename T> class DataGenerator {
public:
	DataGenerator();
	T operator()();
};
template<typename T> void GenerateRandom(vector<T>& res, size_t amount);
template<typename T> void GenerateReversed(vector<T>& res, size_t amount);
template<typename T> void GenerateFewUnique(vector<T>& res, size_t amount);
template<typename T> void GenerateNearlySorted(vector<T>& res, size_t amount);

// Global rand
std::mt19937_64 mt64((uint64_t)time(nullptr) * 2);

string binaryOutput = "";

// ------------------------------------------------------------------------------

void PrintHelp()
{
	printf("Arguments: N [, DataType [, Arrangement]] [option...]\n");
	printf("    DataType can be:    i32, u32, i64, u64, f64\n");
	printf("    Arrangement can be: random, reversed, fewunique, nsorted\n");
	printf("    Option can be:\n");
	printf("        -b file         Output as binary to file\n");
}
int main(int argc, char** argv)
{
	if (argc < 2) {
		PrintHelp();
		return 0;
	}

	{
		OptParse optParse(argc, argv);
		if (optParse.OptionExists("-b")) {
			if (auto file = optParse.GetOptionParam("-b")) {
				binaryOutput = *file;
			}
			else {
				printf("-b: File name is required\n");
				return -1;
			}
		}
	}

	uint64_t countData = std::strtoull(argv[1], nullptr, 10);
	DataType typeDataGenerate = DataType::f64;
	DataArrangeType typeDataArrangement = DataArrangeType::Random;

	if (argc > 2) {
		typeDataGenerate = GetDataTypeFromString(argv[2]);
	}
	if (argc > 3) {
		typeDataArrangement = GetDataArrangeTypeFromString(argv[3]);
	}

	if (typeDataGenerate == DataType::Invalid || typeDataArrangement == DataArrangeType::Invalid) {
		PrintHelp();
		return 0;
	}

	try {
		GenerateDataFromType(countData, typeDataGenerate, typeDataArrangement);
	}
	catch (const string& e) {
		printf("Fatal error-> %s\n", e.c_str());
	}

	//printf("\n");
	return 0;
}

void GenerateDataFromType(size_t count, DataType type, DataArrangeType arrangement)
{
	switch (type) {
	case DataType::i32:
		GenerateDataFromArrangement<int32_t>(count, arrangement);
		break;
	case DataType::u32:
		GenerateDataFromArrangement<uint32_t>(count, arrangement);
		break;
	case DataType::i64:
		GenerateDataFromArrangement<int64_t>(count, arrangement);
		break;
	case DataType::u64:
		GenerateDataFromArrangement<uint64_t>(count, arrangement);
		break;
	case DataType::f64:
		GenerateDataFromArrangement<double_t>(count, arrangement);
		break;
	default: break;
	}
}

// ------------------------------------------------------------------------------

template<> class DataGenerator<int32_t> {
	std::mt19937 mt;
public:
	DataGenerator() : mt((uint64_t)time(nullptr)) {}
	int32_t operator()() { return mt(); }
};
template<> class DataGenerator<uint32_t> {
	std::mt19937 mt;
public:
	DataGenerator() : mt((uint64_t)time(nullptr)) {}
	uint32_t operator()() { return mt(); }
};
template<> class DataGenerator<int64_t> {
	std::mt19937_64 mt;
public:
	DataGenerator() : mt((uint64_t)time(nullptr)) {}
	int64_t operator()() { return mt(); }
};
template<> class DataGenerator<uint64_t> {
	std::mt19937_64 mt;
public:
	DataGenerator() : mt((uint64_t)time(nullptr)) {}
	uint64_t operator()() { return mt(); }
};
template<> class DataGenerator<double_t> {
	std::mt19937_64 mt;
	std::uniform_real_distribution<> dis;
public:
	DataGenerator() : mt((uint64_t)time(nullptr)), dis(-10000.0, 10000.0) {}
	double_t operator()() { return dis(mt); }
};

template<typename T> void GenerateDataFromArrangement(size_t count, DataArrangeType arrangement)
{
	vector<T> data;
	data.reserve(count);

	switch (arrangement) {
	case DataArrangeType::Random:
		GenerateRandom<T>(data, count);
		break;
	case DataArrangeType::Reversed:
		GenerateReversed<T>(data, count);
		break;
	case DataArrangeType::FewUnique:
		GenerateFewUnique<T>(data, count);
		break;
	case DataArrangeType::NearlySorted:
		GenerateNearlySorted<T>(data, count);
		break;
	default: break;
	}
	
	{
		if (binaryOutput.empty()) {
			for (const T& i : data) {
				std::cout << i << " ";
			}
		}
		else {
			std::ofstream fout(binaryOutput, std::ios::binary);
			if (!fout.is_open()) throw string("Failed to open file for writing");
			
			{
				constexpr size_t MAX_PER_IT = 10000000;
				
				size_t remain = count;
				size_t read = 0;
				do {
					size_t now = std::min(remain, MAX_PER_IT);

					fout.write((const char*)(&data[read]), sizeof(T) * now);
					fout.flush();
					
					remain -= now;
					read += now;
				} while (remain > 0);
			}
			
			fout.close();
		}
	}
}

template<typename T> void GenerateRandom(vector<T>& res, size_t amount)
{
	DataGenerator<T> generator;

	// Generate data
	for (size_t i = 0; i < amount; ++i) {
		res.push_back(generator());
	}
};
template<typename T> void GenerateReversed(vector<T>& res, size_t amount)
{
	DataGenerator<T> generator;

	// Generate data
	for (size_t i = 0; i < amount; ++i) {
		res.push_back(generator());
	}

	// Sort descending
	std::sort(std::execution::par, res.begin(), res.end(), std::greater<T>());
};
template<typename T> void GenerateFewUnique(vector<T>& res, size_t amount)
{
	DataGenerator<T> generator;

	// Guarantee at least 2 uniques
	size_t countUnique = std::max<size_t>(2, amount * Opt_FewUnique_UniquePercentage);

	// Generate array where len(tmp) < len(res)
	vector<T> tmp;
	tmp.reserve(countUnique);
	for (size_t i = 0; i < countUnique; ++i) {
		tmp.push_back(generator());
	}

	// Randomly choose from tmp to fill into output array
	for (size_t i = 0; i < amount; ++i) {
		res.push_back(mt64() % countUnique);
	}
};
template<typename T> void GenerateNearlySorted(vector<T>& res, size_t amount)
{
	DataGenerator<T> generator;

	// Guarantee at least 1 swap
	size_t countSwap = std::max<size_t>(1, amount * Opt_NSorted_SwapPercentage);

	// Generate data
	for (size_t i = 0; i < amount; ++i) {
		res.push_back(generator());
	}

	// Sort ascending
	std::sort(std::execution::par, res.begin(), res.end(), std::less<T>());
	
	// Then randomly swap some elements
	for (size_t i = 0; i < countSwap; ++i) {
		size_t swapA = mt64() % amount;
		size_t swapB = mt64() % amount;
		std::swap(res[swapA], res[swapB]);
	}
};
