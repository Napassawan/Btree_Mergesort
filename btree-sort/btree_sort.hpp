#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <queue>

#include <omp.h>

// ------------------------------------------------------------------------------

//#define USE_STD_SET
//#define USE_STD_HEAP

#ifdef USE_STD_SET
	#include <set>
	template<typename T> using set_t = std::set<T>;
#else
	#include "cpp-btree/btree/set.h"
	template<typename T> using set_t = btree::set<T>;
#endif

#include "algo.hpp"

// ------------------------------------------------------------------------------

namespace btreesort {
#ifdef USE_STD_HEAP
	template<typename ValType, typename Comparator> class MultiwaySet {
		struct comp_reverse {
			constexpr bool operator()(const ValType& x, const ValType& y) const
			{
				return !Comparator()(x, y);
			}
		};
		std::priority_queue<ValType, std::vector<ValType>, comp_reverse> heap;
	public:
		MultiwaySet() = default;

		void Push(const ValType& v) { heap.push(v); }
		void Push(ValType&& v) { heap.push(std::move(v)); }

		bool Empty() const { return heap.empty(); }

		const ValType& Peek() const { return heap.top(); }
		ValType Pop()
		{
			ValType val = std::move(Peek());
			heap.pop();
			return val;
		}
	};
#else
	template<typename ValType, typename Comparator> class MultiwaySet {
		btree::multiset<ValType, Comparator> heap;
	public:
		MultiwaySet() = default;

		void Push(const ValType& v) { heap.insert(v); }
		void Push(ValType&& v) { heap.insert(std::move(v)); }

		bool Empty() const { return heap.empty(); }

		const ValType& Peek() const { return *heap.begin(); }
		ValType Pop() {
			ValType val = std::move(Peek());
			heap.erase(heap.begin());
			return val;
		}
	};
#endif
}

// ------------------------------------------------------------------------------

namespace btreesort {
	struct Settings {
		size_t nProcessors;
		size_t nSubBuckets;
		size_t nMinPerSlice;
		size_t nParallelCutoff;
		size_t nQuickSortCutoff;
		size_t nMaxHeapSize;

		Settings();
		
		static const Settings& get();
	};
	
	// ------------------------------------------------------------------------------

	template<typename Iter, typename Comparator>
	class BTreeSort {
	public:
		using IterVal = typename std::iterator_traits<Iter>::value_type;
		using IterPair = std::array<Iter, 2>;
		
		class SliceBase {
		public:
			size_t id;
			IterPair range;
			size_t count;
			
			SliceBase() = default;
			SliceBase(size_t id, IterPair range) : id(id), range(range)
			{
				count = std::distance(range[0], range[1]);
			}
			
			size_t size() const { return count; }
			IterVal get(size_t i) const { return *(range[0] + i); }
		};
		class Slice : public SliceBase {
		public:
			IterVal median;
			
			Slice() : SliceBase() {}
			Slice(size_t id, IterPair range) : SliceBase(id, range)
			{
				median = this->get(this->size() / 2);
			}
			
			//bool operator<(const Slice& o) const { return median < o.median; }
			//bool operator==(const Slice& o) const { return median == o.median; }
			bool operator<(const Slice& o) const
			{
				if (median != o.median)
					return Comparator()(median, o.median);
				return this->id < o.id;
			}
			bool operator==(const Slice& o) const
			{
				if (median != o.median)
					return false;
				return this->id == o.id;
			}
		};
		class SliceValue {
		public:
			const SliceBase* pSlice;
			size_t iRead;
			
			SliceValue() = default;
			SliceValue(const SliceBase* pSlice) : pSlice(pSlice), iRead(0) {}
			
			IterVal get() const { return pSlice->get(iRead); }
			
			bool operator<(const SliceValue& o) const { return Comparator()(get(), o.get()); }
			bool operator==(const SliceValue& o) const { return get() == o.get(); }
		};
	private:
		IterPair data;
		
		set_t<Slice> setSlices;
	public:
		BTreeSort(Iter begin, Iter end);
		BTreeSort(Iter begin, Iter end, Comparator comp);
		virtual ~BTreeSort();
		
		void Sort();
	private:
		std::vector<std::array<size_t, 3>> _GenerateDivisions(size_t count, size_t divs);
		
		void _SortBucket(size_t id, IterPair range);
		void _ShuffleSlices(Iter dest, const std::vector<const Slice*>& slices);
		void _MultiwayHeap(Iter dest, const std::vector<SliceBase>& slices);
	};

	// ------------------------------------------------------------------------------

#define TEMPL template<typename Iter, typename Comparator>
#define DEF_BTreeSort BTreeSort<Iter, Comparator>::

	TEMPL inline DEF_BTreeSort 
	BTreeSort(Iter begin, Iter end) : 
		data({ begin, end }) 
	{
		omp_set_dynamic(false);
		omp_set_num_threads(Settings::get().nProcessors);
	}
	TEMPL inline DEF_BTreeSort
	BTreeSort(Iter begin, Iter end, Comparator comp) :
		BTreeSort(begin, end) {}
	TEMPL inline DEF_BTreeSort ~BTreeSort() {}
	
	TEMPL void DEF_BTreeSort Sort()
	{
		size_t nProcessors = Settings::get().nProcessors;
		//size_t nSlices = Settings::get().nSubBuckets;
		
		auto& [itrBegin, itrEnd] = data;
		size_t dataCount = std::distance(itrBegin, itrEnd);
		
		// If too few data, just use normal sorting
		if (dataCount < Settings::get().nParallelCutoff) {
			std::sort(itrBegin, itrEnd, Comparator());
		}
		else {
			auto buckets = _GenerateDivisions(dataCount, nProcessors);
			
#pragma omp parallel for
			for (auto& [i, begin, end] : buckets) {
				_SortBucket(i, { itrBegin + begin, itrBegin + end });
			}
			
			{
				std::vector<const Slice*> slicesSorted;
				slicesSorted.reserve(setSlices.size());
				
				for (const Slice& s : setSlices) {
					slicesSorted.push_back(&s);
				}
				
				_ShuffleSlices(itrBegin, slicesSorted);
				bs_InsertionSort(itrBegin, itrEnd, Comparator());
			}
		}
	}
	
	// Divides [count] elements into [divs] divisions roughly equally
	TEMPL std::vector<std::array<size_t, 3>> DEF_BTreeSort 
	_GenerateDivisions(size_t count, size_t divs)
	{
		std::vector<std::array<size_t, 3>> res { divs };
		
		for (size_t i = 0; i < divs; ++i) {
			size_t begin = count * i / divs;
			size_t end = count * (i + 1) / divs;
			res[i] = { i, begin, end };
		}
		
		return res;
	}
	
	TEMPL void DEF_BTreeSort _SortBucket(size_t id, IterPair bucket)
	{
		auto& [itrBegin, itrEnd] = bucket;
		size_t count = std::distance(itrBegin, itrEnd);
		
		size_t heapSize = Settings::get().nMaxHeapSize;
		bs_QuickSort<false>(itrBegin, itrEnd, Comparator(), heapSize);
		
		size_t nSlices = count / heapSize;
		if (nSlices < Settings::get().nSubBuckets)
			nSlices = Settings::get().nSubBuckets;
		
		auto partitions = _GenerateDivisions(std::distance(itrBegin, itrEnd), nSlices);
		
#pragma omp critical
		{
			// Partition slices
			
			for (auto& [i, begin, end] : partitions) {
				Slice s(setSlices.size(), { itrBegin + begin, itrBegin + end });
				setSlices.insert(std::move(s));
			}
		}
	}
	TEMPL void DEF_BTreeSort _ShuffleSlices(Iter dest, 
		const std::vector<const Slice*>& slicesSorted)
	{
		size_t nProcessors = Settings::get().nProcessors;
		
		{
			struct _ShufParam {
				size_t index;
				
				std::vector<IterVal> tmp;
				std::vector<SliceBase> newSlices;
				
				std::array<size_t, 2> srcRange;

				size_t placement;
				size_t count;
			};
			std::vector<_ShufParam> shufParams(nProcessors);
			
			auto sliceDivs = _GenerateDivisions(slicesSorted.size(), shufParams.size());
			
			{
				size_t placement = 0;
				for (auto& [i, iBegin, iEnd] : sliceDivs) {
					auto itrSliceBeg = slicesSorted.begin() + iBegin;
					auto itrSliceEnd = slicesSorted.begin() + iEnd;
					
					// Sum the amount of all data in this slice range
					size_t count = 0;
					for (auto itr = itrSliceBeg; itr != itrSliceEnd; ++itr) {
						count += (*itr)->size();
					}
					
					_ShufParam sp {};
					sp.index = i;
					sp.srcRange = { iBegin, iEnd };
					sp.placement = placement;
					sp.count = count;
					
					shufParams[i] = std::move(sp);
					placement += count;
				}
			}

//#pragma omp parallel
			{
#pragma omp parallel for
				for (_ShufParam& sp : shufParams) {
					sp.tmp.reserve(sp.count);
					
					for (auto i = sp.srcRange[0]; i != sp.srcRange[1]; ++i) {
						const Slice* s = slicesSorted[i];
						
						// Copy data from original slice into temp array
						auto before = sp.tmp.insert(sp.tmp.end(), 
							s->range[0], s->range[1]);
						
						// Copy slice info, but change the range
						SliceBase ns(s->id, { before, sp.tmp.end() });
						sp.newSlices.push_back(std::move(ns));
					}
				}

//#pragma omp barrier

#pragma omp parallel for
				for (const _ShufParam& sp : shufParams) {
					_MultiwayHeap(dest + sp.placement, sp.newSlices);
					//std::copy(sp.tmp.begin(), sp.tmp.end(), dest + sp.placement);
				}
			}
		}
	}
	TEMPL void DEF_BTreeSort _MultiwayHeap(Iter dest, const std::vector<SliceBase>& slices)
	{
		MultiwaySet<SliceValue, std::less<SliceValue>> heap;
		
		for (auto& s : slices) {
			if (s.size() > 0) {
				heap.Push(SliceValue(&s));
			}
		}

		while (!heap.Empty()) {
			SliceValue front = std::move(heap.Peek());
			
			// Pop min element from heap into dest
			*(dest++) = front.get();
			heap.Pop();
			
			{
				// Advance read index and move the next element into the heap
				
				front.iRead += 1;
				if (front.iRead < front.pSlice->size()) {
					heap.Push(std::move(front));
				}
			}
		}
	}
	
#undef TEMPL
	
	// ------------------------------------------------------------------------------
	
	Settings::Settings()
	{
		nProcessors = omp_get_num_procs();
		nSubBuckets = nProcessors;
		
		/* nProcessors = 4;
		nSubBuckets = 4; */
		
		nMinPerSlice = 4;
		nParallelCutoff = nProcessors * nSubBuckets * nMinPerSlice;
		
		nMaxHeapSize = 512;
		nQuickSortCutoff = 1024;
	}
	const Settings& Settings::get()
	{
		static Settings s {};
		return s;
	}
}
