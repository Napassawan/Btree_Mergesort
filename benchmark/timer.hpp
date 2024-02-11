#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <exception>

#ifndef WINDOWS
	#if defined(_WIN32) || defined(_WIN64)
		#define WINDOWS
	#endif
#endif

#ifdef WINDOWS
	//#define _WIN32_WINNT 0x0601
	#define WIN32_LEAN_AND_MEAN
	
	#include <windows.h>
	#include <winternl.h>
#else
	#include <unistd.h>
#endif

#ifdef WINDOWS

#else

#endif

class PerformanceTimer {
	struct CpuData {
		uint64_t total;
		uint64_t user;
		uint64_t sys;
		uint64_t idle;

		CpuData operator-(const CpuData& obj);
	};
	struct Stat {
		CpuData total_;
		std::vector<CpuData> cores_;

		Stat operator-(const Stat& obj);
	};

	uint32_t countCPU_;

	Stat start_;
	Stat end_;
	bool bRunning_;

	void _CollectCpuStat(Stat* out);

public:
	PerformanceTimer();
	~PerformanceTimer();

	void Start();
	void Stop();

	void Report(std::ostream& out, bool compact = false, bool verbose = false);
};

class MyException : public std::runtime_error {
	std::string msg_;
public:
	MyException() : MyException("") {}
	MyException(const char* msg) : MyException(std::string(msg)) {}
	MyException(const std::string& msg) : runtime_error(msg) { msg_ = msg; }

	const std::string& GetMessage() { return msg_; }
	const char* what() { return msg_.c_str(); }
};