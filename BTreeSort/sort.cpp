#include "sort.hpp"

#define TEMPL template<typename Iter, typename Comparator>

#define BTREE BTreeSort<Iter, Comparator>

/* TEMPL bool operator<(const typename BTREE::Slice& r, const typename BTREE::Slice& l) {
	return r.median < l.median;
}
TEMPL bool operator==(const typename BTREE::Slice& r, const typename BTREE::Slice& l) {
	return r.median < l.median;
} */
