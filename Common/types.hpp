#pragma once

#include <string.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#else
	#define strcmpi strcasecmp
#endif

enum class DataType {
	i32,
	u32,
	i64,
	u64,
	f64,
	Invalid,
};
static DataType GetDataTypeFromString(char* type) {
#define CHECK(_chk, _res) if (strcmpi(type, _chk) == 0) return _res
	
	CHECK("i32", DataType::i32);
	else CHECK("u32", DataType::u32);
	else CHECK("i64", DataType::i64);
	else CHECK("u64", DataType::u64);
	else CHECK("f64", DataType::f64);
	else CHECK("double", DataType::f64);
	
	return DataType::Invalid;

#undef CHECK
}

enum class DataArrangeType {
	Random,
	Reversed,
	FewUnique,
	NearlySorted,
	Invalid,
};
static DataArrangeType GetDataArrangeTypeFromString(char* type) {
#define CHECK(_chk, _res) if (strcmpi(type, _chk) == 0) return _res
	
	CHECK("random", DataArrangeType::Random);
	else CHECK("ran", DataArrangeType::Random);
	
	else CHECK("rev", DataArrangeType::Reversed);
	else CHECK("reverse", DataArrangeType::Reversed);
	else CHECK("reversed", DataArrangeType::Reversed);
	
	else CHECK("few", DataArrangeType::FewUnique);
	else CHECK("fewunique", DataArrangeType::FewUnique);
	
	else CHECK("nsort", DataArrangeType::NearlySorted);
	else CHECK("nsorted", DataArrangeType::NearlySorted);
	
	return DataArrangeType::Invalid;

#undef CHECK
}

enum class SortType {
	MultiwayMerge,
	BalancedQuick,
	BTreeMerge,
	Invalid,
};
static SortType GetSortTypeFromString(char* type) {
#define CHECK(_chk, _res) if (strcmpi(type, _chk) == 0) return _res
	
	CHECK("mw", SortType::MultiwayMerge);
	else CHECK("multiway", SortType::MultiwayMerge);
	
	else CHECK("bq", SortType::BalancedQuick);
	else CHECK("balanced", SortType::BalancedQuick);
	
	else CHECK("bt", SortType::BTreeMerge);
	else CHECK("btree", SortType::BTreeMerge);
	
	return SortType::Invalid;

#undef CHECK
}
