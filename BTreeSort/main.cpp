#include <iostream>
#include <string.h>

//#include "set.h"

#include "timer.hpp"

#ifdef WINDOWS
	#pragma message("Compile mode: Windows")
#else
	#pragma message("Compile mode: UNIX")
#endif

void PrintHelp() {
	printf("Arguments: N [, DataType [, Arrangement]]\n");
	printf("    DataType can be:    i32, u32, i64, u64, f64\n");
	printf("    Arrangement can be: random, reversed, fewunique, nsorted\n");
}
int main(int argc, char** argv) {
	//std::cout << "Dragon" << std::endl;
	
	bool bCompact = false;
	bool bVerbose = false;
	if (argc > 1) {
		if (strchr(argv[1], 'c'))
			bCompact = true;
		if (strchr(argv[1], 'v'))
			bVerbose = true;
	}
	
	PerformanceTimer timer;
	
	timer.Start();
	
	{
		double a = 43.3223895;
		for (int i = 0; i < 10000000; ++i) {
			a *= 1.10953;
			a -= 3.15221;
		}
	}
	
	timer.Stop();
	
	timer.Report(std::cout, bCompact, bVerbose);
	
	//printf("Done");
	return 0;
}
