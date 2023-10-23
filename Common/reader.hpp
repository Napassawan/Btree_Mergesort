#pragma once

#include <string>
#include <vector>

class FileReader {
public:
	std::string path;
	bool binary;
public:
	FileReader();
	FileReader(const std::string& path, bool binary);
	
	template<typename T> std::vector<T> ReadData() const;
};
