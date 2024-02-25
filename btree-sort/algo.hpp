#pragma once

#include <vector>
#include <algorithm>

#include <omp.h>

namespace btreesort {
	template<typename Iter, typename Pred>
	Iter bs_Partition(Iter begin, Iter end, Pred pred)
	{
		auto i = begin;
		for (auto j = begin; j != end; ++j) {
			if (pred(*j)) {
				std::swap(*i++, *j);
			}
		}
		return i;
	}
	
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
	void bs_InsertionSort(Iter begin, Iter end, Comparator comp)
	{
		if (begin == end) return;
		
		for (auto i = begin + 1; i != end; ++i) {
			auto val = *i;
			if (comp(val, *begin)) {
				std::copy_backward(begin, i, i + 1);
				*begin = val;
			}
			else {
				bs_UnguardedLinearInsert(i, val, comp);
			}
		}
	}
}