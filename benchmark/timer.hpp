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

		CpuData operator+(const CpuData& obj) const;
		CpuData operator-(const CpuData& obj) const;
	};
	struct Stat {
		CpuData total_;
		std::vector<CpuData> cores_;

		Stat operator+(const Stat& obj) const;
		Stat operator-(const Stat& obj) const;
	};

	uint32_t countCPU_;
	
	std::vector<Stat> statsSaved_;

	Stat statBegin_;
	bool bRunning_;

	Stat _CollectCpuStat();
public:
	PerformanceTimer();
	~PerformanceTimer();

	void Start();
	Stat Stop();
	
	void AddDataPoint(const Stat& st);

	void Report(std::ostream& out, bool compact = false, bool verbose = false) const;
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