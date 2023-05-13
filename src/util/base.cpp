#include "base.hpp"


uint8_t  	   __run_once = 0;
uint16_t 	   __finished = 0;
std::uintptr_t __out 	  = 0;

#if defined(_DEBUG)
	std::atomic<size_t> markflag{0};
#endif