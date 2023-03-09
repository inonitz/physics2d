#pragma once
#include "vec.hpp"




using Vec3 = math::vec3f;
using Vec4 = math::vec4f;
using Mat4 = math::mat4f;
using Pressure = f32;

struct GridCell {
	Vec3 	 pos; /* needs to have integer indices :/ */
	Vec3 	 vel;
	Vec3 	 tmp;
	Pressure p;
	u8 layer;
	u8 type; /* Air, Fluid, Solid. */
};







struct Fluid
{
	f32 dx;
	f32 u_max;
	f32 k_cfl;
	f32 dt;	
};