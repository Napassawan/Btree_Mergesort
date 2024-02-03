#pragma once

#include <string>

template<typename Iter>
static std::string StringJoin(Iter begin, Iter end, const std::string& delim)
{
	std::string res = "";
	for (auto itr = begin; itr != end; ++itr) {
		res += *itr;
		if (std::next(itr) != end)
			res += delim;
	}
	return res;
}

template<typename Iter, typename Pred>
static std::string StringJoin(Iter begin, Iter end, const std::string& delim, Pred pred)
{
	std::string res = "";
	for (auto itr = begin; itr != end; ++itr) {
		res += pred(*itr);
		if (std::next(itr) != end)
			res += delim;
	}
	return res;
}
