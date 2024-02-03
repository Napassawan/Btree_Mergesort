#pragma once

#include <omp.h>

// RAII for omp_lock_t
class OmpLock {
	omp_lock_t* pLock;
public:
	OmpLock(omp_lock_t& lock) {
		pLock = &lock;
		omp_set_lock(pLock);
	}
	~OmpLock() { 
		omp_unset_lock(pLock);
	}
};
