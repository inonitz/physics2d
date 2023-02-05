#pragma once
#include "vec.hpp"




struct Camera 
{
private:
	const math::vec3f worldUpDirection = {0.0f, 1.0f, 0.0f };
	f32 fov, aspectRatio, nearClip, farClip;
	f32 cameraSpeed = 5.0f, cameraRotationSpeed = 1.0f;

	math::vec3f position;
	math::vec3f front;
	math::vec2f aggregateAngles;

	math::mat4f proj, view;
public:
	void create(
		math::vec3f const& cameraPosition,
		math::vec4f        projectionData /* fov, aspectRatio, near, far */	
	);


	void onUpdate(double dt);
	void onResize(i32 width, i32 height);
	void updateProjection(math::vec4f const& projectionData);

	
	math::mat4f const& projectionMatrix() const { return proj; }
	math::mat4f const& viewMatrix() 	  const { return view; }

private:
	void recalculateProjection();
	void recalculateView();
};