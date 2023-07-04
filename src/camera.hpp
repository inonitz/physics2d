#pragma once
#include "vec.hpp"




struct Camera 
{
private:
	const math::vec3f worldUp = { 0.0f, 1.0f, 0.0f };
	math::vec3f pos;
	math::vec3f front;
	math::vec2f rotation;
	math::mat4f view;

public:
	math::vec2f velocity; /* velocity[0] = dx/dt. velocity[1] = d0/dt, Rotation Speed (Angular Velocity) */


	void create(
		math::vec3f const& initialPosition, 
		math::vec3f const& frontDirection = { 0.0f, 0.0f, 1.0f }, 
		math::vec2f const& speeds 		  = { 25.0f, 3.0f } /* speeds = { movementSpeed, rotationSpeed } */
	);

	void onUpdate(f32 dt);


	__force_inline math::mat4f const& constref() const { return view;  }
	__force_inline math::mat4f const* constptr() const { return &view; }
	__force_inline math::vec3f const& position() const { return pos;   }
	__force_inline math::vec3f       lookingAt() const { return pos + front; }

private:
	__force_inline void recalcView() {
		math::lookAt(pos, pos + front, worldUp, view);
		return;
	}
};




struct ProjectionMatrix 
{
private:
	math::mat4f data;

	struct underlying_names 
	{
		f32 aspectRatio;
		f32 fieldOfView;
		f32 nearClip;
		f32 farClip;
	};
public:
	union 
	{
		underlying_names __;
		math::vec4f 	 parameters;
	};
	
	
	ProjectionMatrix() : parameters{} { math::identity(data); return; }


	void create(math::vec4f const& initialParamters)
	{
		parameters = initialParamters; 
		recalculate();
		return;
	}


	__force_inline void recalculate() 
	{ 
		math::perspective(
			__.aspectRatio, 
			__.fieldOfView, 
			__.nearClip, 
			__.farClip, 
			data
		); 
		return;
	}
	__force_inline math::mat4f const& constref() const { return data;  }
	__force_inline math::mat4f const* constptr() const { return &data; }
};