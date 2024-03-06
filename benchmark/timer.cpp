#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <inttypes.h>

#include "timer.hpp"

PerformanceTimer::PerformanceTimer()
{
#ifdef WINDOWS
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	countCPU_ = sysInfo.dwNumberOfProcessors;
#else
	countCPU_ = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	bRunning_ = false;
}
PerformanceTimer::~PerformanceTimer() {}

PerformanceTimer::Stat PerformanceTimer::_CollectCpuStat()
{
	Stat res;

#ifdef WINDOWS

	std::vector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> spt(countCPU_);
	NTSTATUS status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
		(PVOID)spt.data(), sizeof(spt[0]) * countCPU_, nullptr);
	
	if (status != 0)
		throw MyException("NtQuerySystemInformation failed");

	// spt values are in 100ns intervals

	constexpr uint64_t CLK_DIV = 10'000'000u / CLOCKS_PER_SEC;
	
	res.total_ = { 0, 0, 0, 0 };
	res.cores_.resize(countCPU_);

	for (size_t i = 0; i < countCPU_; ++i) {
		uint64_t sys = (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) / CLK_DIV,
		         user = spt[i].UserTime.QuadPart / CLK_DIV,
		         idle = spt[i].IdleTime.QuadPart / CLK_DIV;

		res.total_.sys += (res.cores_[i].sys = sys);
		res.total_.user += (res.cores_[i].user = user);
		res.total_.idle += (res.cores_[i].idle = idle);
		res.total_.total += (res.cores_[i].total = sys + user + idle);
	}

#else

	std::ifstream stat("/proc/stat");
	if (!stat.is_open()) throw MyException("Open proc/stat failed");

	res.cores_.resize(countCPU_);

	std::string line;
	while (std::getline(stat, line)) {
		std::istringstream ss(line);

		std::string head;
		ss >> head;

		// Line must start with "cpu"
		if (head.find("cpu") == 0) {
			CpuData* target = &res.total_;

			// cpu -> total time
			// cpu0, cpu1, cpu2, ... -> time per core
			if (head.size() > 3) {
				int coreId = strtol(head.c_str() + 3, nullptr, 10);
				target = &res.cores_[coreId];
			}

			auto times = std::vector(
				std::istream_iterator<uint64_t>(ss),
				std::istream_iterator<uint64_t>());

			target->user = times[0];
			auto nice = times[1];
			target->sys = times[2];
			target->idle = times[3];

			target->total = target->user + nice + target->sys + target->idle;
		}
	}

#endif

	return res;
}

void PerformanceTimer::Start()
{
	if (bRunning_)
		throw MyException("Timer already running");
	bRunning_ = true;
	
	statBegin_ = _CollectCpuStat();
}
PerformanceTimer::Stat PerformanceTimer::Stop()
{
	if (!bRunning_)
		throw MyException("Timer not started yet");
	bRunning_ = false;

	auto statNow = _CollectCpuStat();
	return (statNow - statBegin_);
}
void PerformanceTimer::AddDataPoint(const Stat& st)
{
	statsSaved_.push_back(std::move(st));
}

void PerformanceTimer::Report(std::ostream& out, bool compact, bool verbose) const
{
	if (bRunning_)
		throw MyException("Timer not stopped yet");

	auto _Print = [&](int core, const CpuData& time) {
		char tmp[256];

		if (core >= 0) {
			sprintf(tmp, "Core%02d: ", core);
			out << tmp;
		}
		else {
			out << "Total:  ";
		}

		if (!compact) {
			out << "\n";
			out << "    Total time:  " << time.total << "\n";
			out << "    User time:   " << time.user << "\n";
			out << "    System time: " << time.sys << "\n";
			out << "    Idle time:   " << time.idle << "\n";
		}
		else {
			sprintf(tmp, 
				"%10" PRIu64 ", %10" PRIu64 ", %10" PRIu64 ", %10" PRIu64 "\n",
				time.total, time.user, time.sys, time.idle);
			out << tmp;
		}
	};
	auto _PrintStatAll = [&_Print](const Stat& stat) {
		_Print(-1, stat.total_);
		for (size_t i = 0; i < stat.cores_.size(); ++i)
			_Print(i, stat.cores_[i]);
	};
	
	if (verbose)
		out << "Total====================\n";
	{
		Stat stSum{};	// zero-initialize
		stSum.cores_.resize(countCPU_);

		for (const auto& st : statsSaved_) {
			stSum = std::move(stSum + st);
		}

		_PrintStatAll(stSum);
	}
	
	if (verbose) {
		// Also print each data point

		size_t i = 1;
		for (const auto& st : statsSaved_) {
			out << "\nRun" << i << "===================\n";
			_PrintStatAll(st);
			++i;
		}
	}
	
	out << std::endl;
}

PerformanceTimer::CpuData PerformanceTimer::CpuData::operator+(const CpuData& obj) const
{
	return CpuData {
		total + obj.total,
		user + obj.user,
		sys + obj.sys,
		idle + obj.idle,
	};
}
PerformanceTimer::CpuData PerformanceTimer::CpuData::operator-(const CpuData& obj) const
{
	return CpuData {
		total - obj.total,
		user - obj.user,
		sys - obj.sys,
		idle - obj.idle,
	};
}

PerformanceTimer::Stat PerformanceTimer::Stat::operator+(const Stat& obj) const
{
	Stat res;
	res.total_ = total_ + obj.total_;

	size_t nCores = cores_.size();
	if (nCores != obj.cores_.size())
		throw MyException("Core data count mismatch (operator+)");

	res.cores_.resize(nCores);
	for (size_t i = 0; i < nCores; ++i)
		res.cores_[i] = cores_[i] + obj.cores_[i];
	
	return res;
}
PerformanceTimer::Stat PerformanceTimer::Stat::operator-(const Stat& obj) const
{
	Stat res;
	res.total_ = total_ - obj.total_;
	
	size_t nCores = cores_.size();
	if (nCores != obj.cores_.size())
		throw MyException("Core data count mismatch (operator-)");
	
	res.cores_.resize(nCores);
	for (size_t i = 0; i < nCores; ++i)
		res.cores_[i] = cores_[i] - obj.cores_[i];
	
	return res;
}
