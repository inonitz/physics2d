#pragma once
#include "vec.hpp"



struct GridCell {
	u8 type; /* Air, Fluid, Solid. */
};







struct Fluid
{
	f32 dx;
	f32 u_max;
	f32 k_cfl;
	f32 dt;	
};