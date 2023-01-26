#pragma once
#include "vec.hpp"



namespace mt = math;




struct Ray 
{
	mt::point3 pos;
	mt::vec3f  dir;

	Ray() : pos(0.0f), dir(0.0f) {}
	Ray(
		mt::vec3f const& startPos, 
		mt::vec3f const& rayDir
	) : pos(startPos), dir(rayDir) {
		dir.normalize();
		return;
	}


	__force_inline mt::vec3f at(float t) { return pos + t * dir; }
};


struct Sphere {
	mt::point3 center;
	float 	   rsqr;


	mt::vec3f getNormal(mt::vec3f const& initial) 
	{
		mt::vec3f out = initial; 
		out = out - center; 
		out.normalize();
		return out;
	}


	std::array<float, 2> intersection(Ray const& r) 
	{
		std::array<float, 2> rts = { NAN, NAN };
		mt::vec3f omc;
		float a, b, c;

		omc = r.pos; 
		omc = omc - center;
		a = r.dir.lensqr();
		b = mt::dot(r.dir, omc);
		c = omc.lensqr() - rsqr;

		c = abs(sqrt(b*b - a*c)); /* the 4 gets canceled out in the calculation. */
		a = 1.0f / a;
		
		b *= a;
		c *= a;
		rts = {
			b - c, /* (b - root)/a */
			b + c  /* (b + root)/a */
		};
		return rts; /* if the roots are < 0 then the resulting roots will contain NAN. */
	}
};


struct Plane {
	mt::point4 normal;


	__force_inline mt::vec3f getNormal() { return mt::vec3f(normal.begin()); }


	float intersection(Ray const& r) {
		mt::point4 pos;
		mt::vec4f  dir;
		float 	   tmp, t;
		
		pos = r.pos.homogenised; pos.w = 1.0f;
		dir = r.dir.homogenised; dir.w = 0.0f;
		t   = -mt::dot(normal, pos);
		tmp = mt::dot(normal, dir);
		tmp = 1.0f / tmp;
		return t * tmp; /* t = - (normal * pos) / (normal * dir) */
	}
};

