#include "reader.hpp"

#include <fstream>
#include <cstdint>

FileReader::FileReader() : FileReader("", false) {}
FileReader::FileReader(const std::string& path, bool binary) : 
	path(path), binary(binary) {}

// ------------------------------------------------------------------------------

// Explicit template instantiations
#define ITEMPL_ReadData(_ty) template std::vector<_ty> FileReader::ReadData<_ty>() const;

ITEMPL_ReadData(int32_t);
ITEMPL_ReadData(uint32_t);
ITEMPL_ReadData(int64_t);
ITEMPL_ReadData(uint64_t);
ITEMPL_ReadData(double);

template<typename T> std::vector<T> FileReader::ReadData() const
{
	std::ifstream file(path, binary ? std::ios::binary : (std::ios::openmode)0);
	if (!file.is_open())
		throw std::string("Failed to open file for reading");

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<T> res;

	if (binary) {
		if (fileSize % sizeof(T) != 0)
			throw std::string("Wrong file size for data type");

		size_t dataCount = fileSize / sizeof(T);
		res.resize(dataCount);

		// Buffered read into res
		{
			constexpr size_t MAX_PER_IT = 4096;

			size_t pos = 0;
			size_t remain = dataCount;
			while (remain > 0) {
				size_t read = std::min(MAX_PER_IT, remain);
				file.read((char*)&res[pos], read * sizeof(T));
				
				pos += read;
				remain -= read;
			}
		}
	}
	else {
		// Use file exceptions instead of putting checks inside the read loop
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			T value;
			while (file >> value) {
				res.push_back(value);
			}
		}
		catch (const std::ifstream::failure& e) {
			//throw (std::string("File read error: ") + e.what());
		}
	}

	file.close();
	return res;
}