#pragma once
#include "vec.hpp"




struct FPSCamera 
{
	const math::vec3f cameraFront = { 0.0f, 0.0f, 1.0f };
	
	math::vec3f position;
	math::vec3f vel;
	math::vec4f projData;
	
	math::mat4f persp;
	math::mat4f inv_persp;
	math::mat4f view;


	void create(
		math::vec4f const& perspectiveVector, /* fovy, aspect_ratio(w/h), near, far */	
		math::vec3f const& currentPos, 
		math::vec3f const& lookingAtPos,
		math::vec3f const& upVector = { 0.0f, 1.0f, 0.0f }
	);


	void update(double fixedDelta);
};