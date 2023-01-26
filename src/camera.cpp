#include "camera.hpp"




void FPSCamera::create(
	math::vec4f const& perspectiveVector, /* aspect_ratio(w/h), fovy, near, far */	
	math::vec3f const& currentPos, 
	math::vec3f const& lookingAtPos,
	math::vec3f const& upVector
) {
	position = currentPos;
	projData = perspectiveVector;

	math::perspective(
		perspectiveVector[0], 
		perspectiveVector[1], 
		perspectiveVector[2], 
		perspectiveVector[3],  
		persp
	);
	math::lookAt(currentPos, lookingAtPos, upVector, view);
}


void FPSCamera::update(__unused double fixedDelta)
{
	
}