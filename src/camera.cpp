#include "camera.hpp"
#include "callbackHooks.hpp"





void Camera::create(
	math::vec3f const& cameraPosition,
	math::vec4f        projectionData /* fov, aspectRatio, near, far */	
) {
	position = cameraPosition;
	front    = { 0.0f, 0.0f,  1.0f };
	aggregateAngles = { 0.0f, 0.0f };

	fov 		= projectionData[0];
	aspectRatio = projectionData[1];
	nearClip 	= projectionData[2];
	farClip  	= projectionData[3];

	math::identity(proj);
	math::identity(view);
	return;
}




void Camera::onUpdate(double dt)
{
	math::vec2f delta;
	math::vec3f camRight = math::cross(worldUpDirection, front); camRight.normalize();
	if(getKeyState(KeyCode::W) == InputState::PRESS || getKeyState(KeyCode::W) == InputState::REPEAT) { position += front    * (dt * cameraSpeed); }
	if(getKeyState(KeyCode::S) == InputState::PRESS || getKeyState(KeyCode::S) == InputState::REPEAT) { position -= front    * (dt * cameraSpeed); }
	if(getKeyState(KeyCode::D) == InputState::PRESS || getKeyState(KeyCode::D) == InputState::REPEAT) { position -= camRight * (dt * cameraSpeed); }
	if(getKeyState(KeyCode::A) == InputState::PRESS || getKeyState(KeyCode::A) == InputState::REPEAT) { position += camRight * (dt * cameraSpeed); }


	delta = getCursorDelta<f32>();
	delta *= cameraRotationSpeed;
	if(delta.x != 0.0f && delta.y != 0.0f) {
		aggregateAngles += delta;
		aggregateAngles.pitch = std::clamp(aggregateAngles.pitch, -89.0f, 89.0f);
		
		math::vec2f eulerRadians = { 
			math::radians(aggregateAngles.yaw  ), 
			math::radians(aggregateAngles.pitch) 
		};
		front = {
			cosf(eulerRadians.yaw) * cosf(eulerRadians.pitch),
			sinf(eulerRadians.pitch),
			sinf(eulerRadians.yaw) * cosf(eulerRadians.pitch)
		};
		front.normalize();
	}


	recalculateView();
	return;
}


/*
	Don't know about this function, rethink whether you'll actually use it.
*/
void Camera::onResize(i32 width, i32 height)
{
	f32 newRatio = static_cast<f32>(width) * (1.0f / static_cast<f32>(height));
	if(newRatio == aspectRatio)
		return;
	
	aspectRatio = newRatio;
	recalculateProjection();
	return;
}


void Camera::updateProjection(math::vec4f const& projectionData)
{
	fov 		= projectionData[0];
	aspectRatio = projectionData[1];
	nearClip 	= projectionData[2];
	farClip  	= projectionData[3];
	recalculateProjection();
	return;
}



void Camera::recalculateProjection()
{
	math::perspective(
		aspectRatio, 
		math::radians(fov), 
		nearClip, 
		farClip, 
		proj
	);
	return;
}


void Camera::recalculateView()
{
	math::lookAt(
		position, 
		position + front, 
		worldUpDirection,
		view
	);
	return;
}