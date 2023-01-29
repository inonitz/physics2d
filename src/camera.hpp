#pragma once
#include "vec.hpp"




struct FPSCamera 
{
private:
	struct projectionDataVec4f {
		float fovy;
		float aspect_ratio;
		float near;
		float far;
	};
	const math::vec3f worldUp = { 0.0f, 1.0f, 0.0f };
	
	math::vec3f position;
	math::vec3f cameraFront = { 0.0f, 0.0f, 1.0f };
	math::vec3f cameraRight;
	math::vec3f cameraUp;
	 
	math::vec3f vel;
	math::vec4f euler; /* [3] = sensitivity to changes in delta_x, delta_y */
	math::vec4f projData;

	math::mat4f m_persp;
	math::mat4f m_inv_persp;
	math::mat4f m_view;
	math::mat4f m_inv_view;

	bool persp_changed{false};

public:
	void create(
		math::vec4f const& perspectiveVector, /* fovy, aspect_ratio(w/h), near, far */	
		math::vec3f const& currentPos,
		math::vec3f const& frontVector = { 0.0f, 0.0f, 1.0f },
		math::vec3f const& upVector    = { 0.0f, 1.0f, 0.0f },
		math::vec3f const& constVel    = { 1.0f, 1.0f, 1.0f }
	);


	void updateProjection();
	void updateView();
	
	void updateFromKeyboard(double fixedDelta);
	void updateFromMouse();
	void updateProjectionArguments(
		float fieldOfView, 
		float aspectRatio, 
		float nearPlane, 
		float farPlane
	) { 
		projData = { fieldOfView, aspectRatio, nearPlane, farPlane };
		persp_changed = true;
		return;
	}


	math::mat4f const& projection() 		const { return m_persp; 	}
	math::mat4f const& view() 				const { return m_view;  	}
	math::mat4f const& inverse_projection() const { return m_inv_persp; }
	math::mat4f const& inverse_view() 		const { return m_inv_view;  }
private:
	void updateBasisVectors();
};