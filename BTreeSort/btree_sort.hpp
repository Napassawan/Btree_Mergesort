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
	class TopIterBucket : public IterBucket {
	public:
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
	void _InsertionSortRange(Iter begin, Iter end);
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
DEF_BTreeSort TopIterBucket::TopIterBucket(int id, Iter begin, Iter end, size_t nDivisions)
	: IterBucket(id, begin, end)
{
	this->nDivisions = nDivisions;
	maxPerDivision = this->size / nDivisions;
	if (maxPerDivision < 4) {
		maxPerDivision = 4;
		this->nDivisions = this->size / maxPerDivision;
	}
	
	divisions.reserve(nDivisions);
}

TEMPL inline
DEF_BTreeSort BTreeSort(Iter begin, Iter end, Comparator comp) {
	data = IterBucket(-1, begin, end);
	this->comp = comp;
	
	nCores = omp_get_num_procs();
	omp_set_dynamic(0);
	omp_set_num_threads(nCores);

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

#pragma omp parallel for
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
			// TODO: Find a way to not create a whole new array of equal size
			
			std::vector<IterVal> newData;
			newData.reserve(dataCount);
			
			// Copy slices to new vector in ascending order
			for (const Slice& s : setSlices) {
				newData.insert(newData.end(), s.bucket->begin, s.bucket->end);
			}

#pragma omp parallel for
			for (size_t iCore = 0; iCore < nCores; ++iCore) {
				size_t begin = dataCount * iCore / nCores;
				size_t end = dataCount * (iCore + 1) / nCores;
				_InsertionSortRange(
					newData.begin() + begin,
					newData.begin() + end);
			}
			
			for (size_t i = 0; i < dataCount; ++i) {
				*(itrBegin + i) = newData[i];
			}
		}
	}
}
TEMPL void DEF_BTreeSort _WorkBucket(TopIterBucket& bucket) {
	auto& itrBegin = bucket.begin, &itrEnd = bucket.end;
	
	// TODO: Replace with non-STD sort
	std::sort(itrBegin, itrEnd, comp);
	
	// Partition sub-divisions
	// TODO: Maybe make division less naive
	for (size_t iDiv = 0; iDiv < bucket.nDivisions; ++iDiv) {
		size_t begin = bucket.size * iDiv / bucket.nDivisions;
		size_t end = bucket.size * (iDiv + 1) / bucket.nDivisions;
		
		if (end - begin > 0) {
			bucket.divisions.push_back(IterBucket(
				bucket.id * bucket.maxPerDivision + iDiv, 
				itrBegin + begin, itrBegin + end));
		}
	}
	
	{
		OmpLock lock(om_writeLock);		// Lock setSlices for writing
		
		for (IterBucket& div : bucket.divisions) {
			setSlices.insert({ &div, *(div.begin + div.size / 2) });
		}
	}
}
TEMPL void DEF_BTreeSort _InsertionSortRange(Iter begin, Iter end) {
	// Copied from std::__insertion_sort
	
	if (begin == end)
		return;
	
	for (Iter i = begin + 1; i != end; ++i) {
		if (comp(*i, *begin)) {
			IterVal val = std::move(*i);
			
			std::move_backward(begin, i, i + 1);
			*begin = std::move(val);
		}
		else {
			IterVal val = std::move(*i);
			
			Iter next = i;
			--next;
			
			while (comp(val, *next)) {
				*i = std::move(*next);
				i = next;
				--next;
			}

			*i = std::move(val);
		}
	}
}

#undef TEMPL
