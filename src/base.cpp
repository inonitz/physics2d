#include "base.hpp"


uint8_t run_once  = 0;
uint16_t finished = 0;

#ifdef _DEBUG
	std::atomic<size_t> markflag{0};
#endif