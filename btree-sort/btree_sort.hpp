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
	
	template<bool ISORT = true, typename Iter, typename Comparator>
	void bs_QuickSort(Iter begin, Iter end, Comparator comp, size_t cutoff);

	template<typename Iter, typename Comparator>
	void bs_InsertionSort(Iter begin, Iter end, Comparator comp);

	// ------------------------------------------------------------------------------

	template<typename Iter, typename Comparator>
	class BTreeSort {
	public:
		using IterVal = typename std::iterator_traits<Iter>::value_type;
		using IterPair = std::array<Iter, 2>;
		
		// TODO: Clean this mess up
		
		class Slice {
		public:
			size_t id;
			IterPair range;
			size_t count;
			IterVal median;
			Comparator comp;

			Slice() = default;
			Slice(size_t id, IterPair range, Comparator comp) : id(id), range(range), comp(comp)
			{
				count = std::distance(range[0], range[1]);
				median = get(size() / 2);
			}
			
			size_t size() const { return count; }
			IterVal get(size_t i) const { return *(range[0] + i); }
			
			//bool operator<(const Slice& o) const { return median < o.median; }
			//bool operator==(const Slice& o) const { return median == o.median; }
			bool operator<(const Slice& o) const { 
				if (median != o.median)
					return comp(median, o.median);
				return id < o.id;
			}
			bool operator==(const Slice& o) const { 
				if (median != o.median)
					return false;
				return id == o.id;
			}
		};
		
		using SliceRefRange = std::array<typename std::vector<Slice>::const_iterator, 2>;
		
		class SliceValue {
		public:
			const Slice* pSlice;
			Comparator comp;
			size_t iRead;
			
			SliceValue() = default;
			SliceValue(const Slice* pSlice, Comparator comp) : 
				pSlice(pSlice), iRead(0), comp(comp) {}

			IterVal get() const { return pSlice->get(iRead); }
			
			bool operator<(const SliceValue& o) const { return comp(get(), o.get()); }
			bool operator==(const SliceValue& o) const { return get() == o.get(); }
		};
	private:
		IterPair data;
		Comparator comp;
		
		set_t<Slice> setSlices;
		
		omp_lock_t om_writeLock;
	public:
		BTreeSort(Iter begin, Iter end, Comparator comp);
		~BTreeSort();
		
		void Sort();
	private:
		std::vector<std::array<size_t, 3>> _GenerateDivisions(size_t count, size_t divs);
		
		void _SortBucket(size_t id, IterPair range);
		void _ShuffleSlices(Iter dest, const std::vector<Slice>& slices);
		void _MultiwayHeap(Iter dest, SliceRefRange slices);
	};

	// ------------------------------------------------------------------------------

#define TEMPL template<typename Iter, typename Comparator>
#define DEF_BTreeSort BTreeSort<Iter, Comparator>::

	TEMPL inline DEF_BTreeSort 
	BTreeSort(Iter begin, Iter end, Comparator comp) :
		data({ begin, end }), comp(comp) 
	{
		omp_set_dynamic(false);
		omp_set_num_threads(Settings::get().nProcessors);
		
		omp_init_lock(&om_writeLock);
	}
	TEMPL inline DEF_BTreeSort ~BTreeSort()
	{
		omp_destroy_lock(&om_writeLock);
	}
	
	TEMPL void DEF_BTreeSort Sort()
	{
		size_t nProcessors = Settings::get().nProcessors;
		//size_t nSlices = Settings::get().nSubBuckets;
		
		auto& [itrBegin, itrEnd] = data;
		size_t dataCount = std::distance(itrBegin, itrEnd);
		
		// If too few data, just use normal sorting
		if (dataCount < Settings::get().nParallelCutoff) {
			std::sort(itrBegin, itrEnd, comp);
		}
		else {
			auto buckets = _GenerateDivisions(dataCount, nProcessors);

#pragma omp parallel for
			for (auto& [i, begin, end] : buckets) {
				_SortBucket(i, { itrBegin + begin, itrBegin + end });
			}
			
			{
				std::vector<IterVal> dataNew;
				dataNew.reserve(dataCount);
				
				// Load slices from min to max into the new data
				
				std::vector<Slice> slicesSorted;
				slicesSorted.reserve(setSlices.size());
				
				{
					size_t i = 0;
					for (const Slice& s : setSlices) {
						auto before = dataNew.end();
						
						dataNew.insert(dataNew.end(), s.range[0], s.range[1]);
						
						Slice ns(s.id, { before, dataNew.end() }, comp);
						slicesSorted.push_back(std::move(ns));
						
						++i;
					}
				}
				
				_ShuffleSlices(itrBegin, slicesSorted);
				
				bs_InsertionSort(itrBegin, itrEnd, comp);
				
				/* for (size_t i = 0; i < dataCount; ++i) {
					*(itrBegin + i) = dataNew[i];
				} */
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
		bs_QuickSort<false>(itrBegin, itrEnd, comp, heapSize);

		size_t nSlices = count / heapSize;
		if (nSlices < Settings::get().nSubBuckets)
			nSlices = Settings::get().nSubBuckets;

		auto partitions = _GenerateDivisions(std::distance(itrBegin, itrEnd), nSlices);
		
		// Partition slices
		{
			OmpLock lock(om_writeLock); // Lock setSlices for writing
			
			for (auto& [i, begin, end] : partitions) {
				Slice s(setSlices.size(), { itrBegin + begin, itrBegin + end }, comp);
				setSlices.insert(std::move(s));
			}
		}
	}
	TEMPL void DEF_BTreeSort _ShuffleSlices(Iter dest, const std::vector<Slice>& slices)
	{
		size_t nProcessors = Settings::get().nProcessors;
		size_t nSlices = Settings::get().nSubBuckets;
		
		{
			struct _ShufParam {
				size_t index;
				SliceRefRange slices;
				size_t placement;
				size_t count;
			};
			std::vector<_ShufParam> shufParams(nProcessors);
			
			auto sliceDivs = _GenerateDivisions(slices.size(), shufParams.size());
			
			{
				size_t placement = 0;
				for (auto& [i, iBegin, iEnd] : sliceDivs) {
					auto itrSliceBeg = slices.begin() + iBegin;
					auto itrSliceEnd = slices.begin() + iEnd;
					
					// Sum the amount of all data in this slice range
					size_t count = 0;
					for (auto itr = itrSliceBeg; itr != itrSliceEnd; ++itr) {
						count += (*itr).size();
					}
					
					shufParams[i] = _ShufParam {
						i,
						{ itrSliceBeg, itrSliceEnd },
						placement,
						count,
					};
					placement += count;
				}
			}

#pragma omp parallel for
			for (auto& sp : shufParams) {
				_MultiwayHeap(dest + sp.placement, sp.slices);
			}
		}
	}
	TEMPL void DEF_BTreeSort _MultiwayHeap(Iter dest, SliceRefRange slices)
	{
		MultiwaySet<SliceValue, std::less<SliceValue>> heap;
		
		for (auto itr = slices[0]; itr != slices[1]; ++itr) {
			const Slice* s = &*itr;
			if (s->size() > 0) {
				heap.Push(SliceValue(s, comp));
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
		
		/* nProcessors = 8;
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
	
	// ------------------------------------------------------------------------------

	template<bool ISORT, typename Iter, typename Comparator>
	void bs_QuickSort(Iter begin, Iter end, Comparator comp, size_t cutoff)
	{
		if (begin == end) return;
		
		size_t dist = std::distance(begin, end);
		if constexpr (ISORT) {
			if (dist <= cutoff)
				bs_InsertionSort(begin, end, comp);
		}
		
		auto pivot = *std::next(begin, dist / 2);
		
		Iter m1 = std::partition(begin, end, [&](const auto& x) { return comp(x, pivot); });
		Iter m2 = std::partition(m1, end, [&](const auto& x) { return !comp(pivot, x); });

		bs_QuickSort<ISORT>(begin, m1, comp, cutoff);
		bs_QuickSort<ISORT>(m2, end, comp, cutoff);
	}

	// https://github.com/karottc/sgi-stl/blob/b3e4ad93382ac8b47ba1eb8b409917ea1ff8a8b5/stl_algo.h#L1300
	template<typename Iter, 
		typename ValType = typename std::iterator_traits<Iter>::value_type,
		typename Comparator>
	inline void bs_UnguardedLinearInsert(Iter last, ValType val, Comparator comp)
	{
		auto next = last;
		--next;
		
		while (val < *next) {
			*last = *next;
			last = next;
			--next;
		}
		
		*last = val;
	}
	template<typename Iter, typename Comparator>
	inline void bs_LinearInsert(Iter begin, Iter end, Comparator comp)
	{
		auto val = *end;
		if (comp(val, *begin)) {
			std::copy_backward(begin, end, end + 1);
			*begin = val;
		}
		else
			bs_UnguardedLinearInsert(end, val, comp);
	}
	template<typename Iter, typename Comparator>
	void bs_InsertionSort(Iter begin, Iter end, Comparator comp)
	{
		if (begin == end) return;
		
		for (auto i = begin + 1; i != end; ++i)
			bs_LinearInsert(begin, i, comp);
	}
}
