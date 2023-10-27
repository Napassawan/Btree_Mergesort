#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>

#include <omp.h>

#include "cpp-btree/btree/set.h"

template<typename Iter, typename Comparator>
class BTreeSort {
	using IterVal = typename std::iterator_traits<Iter>::value_type;
	using IterPair = std::array<Iter, 2>;
	
	// TODO: Clean this mess up
	
	class IterBucket {
	public:
		int id;
		Iter begin, end;
		size_t size;
	public:
		IterBucket() : IterBucket(-1, {}, {}) {}		// Why is C++ stupid?
		IterBucket(int id, Iter begin, Iter end);
	};
	class TopIterBucket {	// Can't inherit from IterBucket for some reason
	public:
		IterBucket ib;
		size_t nDivisions;
		size_t maxPerDivision;
		std::vector<IterBucket> divisions;
	public:
		TopIterBucket(int id, Iter begin, Iter end, size_t nDivisions);
	};
	
	class Slice {
	public:
		IterBucket* bucket;
		IterVal median;
		
		bool operator<(const Slice& other) const {
			return median < other.median;
		}
		bool operator==(const Slice& other) const {
			return median == other.median;
		}
	};
private:
	IterBucket data;
	Comparator comp;
	
	size_t nCores;
	
	size_t maxPerBucket;
	std::vector<TopIterBucket> buckets;
	
	btree::set<Slice> setSlices;
	
	omp_lock_t om_writeLock;
public:
	BTreeSort(Iter begin, Iter end, Comparator comp);
	~BTreeSort();
	
	void Sort();
private:
	void _WorkBucket(TopIterBucket& bucket);
};

// ------------------------------------------------------------------------------

#define TEMPL template<typename Iter, typename Comparator>
#define DEF_BTreeSort BTreeSort<Iter, Comparator>::

TEMPL inline
DEF_BTreeSort IterBucket::IterBucket(int id, Iter begin, Iter end) {
	this->id = id;
	this->begin = begin;
	this->end = end;
	size = std::distance(begin, end);
}
TEMPL inline
DEF_BTreeSort TopIterBucket::TopIterBucket(int id, Iter begin, Iter end, size_t nDivisions) {
	ib = IterBucket(id, begin, end);
	
	this->nDivisions = nDivisions;
	maxPerDivision = ib.size / nDivisions;
	if (maxPerDivision < 4) {
		maxPerDivision = 4;
		this->nDivisions = ib.size / maxPerDivision;
	}
	
	divisions.reserve(nDivisions);
}

TEMPL inline
DEF_BTreeSort BTreeSort(Iter begin, Iter end, Comparator comp) {
	data = IterBucket(-1, begin, end);
	this->comp = comp;
	
	nCores = omp_get_num_procs();
	
	buckets.clear();
	
	omp_init_lock(&om_writeLock);
}
TEMPL inline
DEF_BTreeSort ~BTreeSort() {
	omp_destroy_lock(&om_writeLock);
}

TEMPL void DEF_BTreeSort Sort() {
	constexpr size_t PARALLEL_CUTOFF = /* 10000 */ 10;
	constexpr size_t N_SUB_BUCKETS = 16;
	
	auto itrBegin = data.begin, itrEnd = data.end;
	size_t dataCount = std::distance(itrBegin, itrEnd);
	
	if (dataCount < PARALLEL_CUTOFF) {
		std::sort(itrBegin, itrEnd);
	}
	else {
		// Partition sub-lists
		for (size_t iCore = 0; iCore < nCores; ++iCore) {
			size_t begin = dataCount * iCore / nCores;
			size_t end = dataCount * (iCore + 1) / nCores;
			buckets.push_back(TopIterBucket(iCore, 
				itrBegin + begin, itrBegin + end, N_SUB_BUCKETS));
		}
		
		for (size_t i = 0; i < nCores; ++i) {
			_WorkBucket(buckets[i]);
		}
		
		/* printf("BTree size: %zu\n", setSlices.size());
		for (const Slice& s : setSlices) {
			const IterBucket& b = *s.bucket;
			IterVal beg = *b.begin, end = *(b.end - 1);
			IterVal mid = s.median;
			std::cout << b.id << " -> " << mid 
				<< " -> [" << beg << ", " << end << "]\n";
		} */
		
		{
			std::vector<IterVal> newData;
			newData.reserve(dataCount);
			
			// Copy slices to new vector in ascending order
			for (const Slice& s : setSlices) {
				newData.insert(newData.end(), 
					s.bucket->begin, s.bucket->end);
			}
			
			{
				std::ofstream fout("tmp.txt");
				for (auto& i : newData) {
					fout << i << " ";
				}
				fout.close();
			}
			
			for (size_t i = 0; i < dataCount; ++i) {
				*(itrBegin + i) = newData[i];
			}
		}
	}
}
TEMPL void DEF_BTreeSort _WorkBucket(TopIterBucket& bucket) {
	auto& itrBegin = bucket.ib.begin, &itrEnd = bucket.ib.end;
	
	// TODO: Replace with non-STD sort
	std::sort(itrBegin, itrEnd, Comparator());
	
	// Partition sub-divisions
	// TODO: Maybe make division less naive
	for (size_t iDiv = 0; iDiv < bucket.nDivisions; ++iDiv) {
		size_t begin = bucket.ib.size * iDiv / bucket.nDivisions;
		size_t end = bucket.ib.size * (iDiv + 1) / bucket.nDivisions;
		
		if (end - begin > 0) {
			bucket.divisions.push_back(IterBucket(
				bucket.ib.id * bucket.maxPerDivision + iDiv, 
				itrBegin + begin, itrBegin + end));
		}
	}
	
	{
		//OmpLock lock(om_writeLock);	// Lock setSlices for writing
		
		/* std::string strDivs = StringJoin(bucket.divisions.begin(), bucket.divisions.end(),
			", ", [](const IterBucket& b) { 
					IterVal beg = *b.begin, end = *b.end;
					IterVal mid = *(b.begin + b.size / 2);
					return "[" + std::to_string(beg) + ", " 
						+ std::to_string(mid) + ", " 
						+ std::to_string(end) + "]";
				});
		std::cout << "Bucket " << bucket.ib.id 
			<< " -> (" << strDivs << ")\n"; */
		
		for (IterBucket& div : bucket.divisions) {
			IterVal mid = *(div.begin + div.size / 2);
			setSlices.insert({ &div, mid });
		}
	}
}

#undef TEMPL
