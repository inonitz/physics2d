#include "camera.hpp"
#include "callbackHooks.hpp"





void Camera::create(
	math::vec3f const& initialPosition, 
	math::vec3f const& frontDirection,
	math::vec2f const& speeds
) {
	pos 	 = initialPosition;
	front    = frontDirection;
	velocity = speeds;

	rotation = {0.0f, 0.0f};
	math::identity(view);
	return;
}


void Camera::onUpdate(f32 dt)
{
	math::vec2f delta, rotRad; 
	math::vec3f right = math::cross(worldUp, front);
	right.normalize();
	if(getKeyState(KeyCode::W) == InputState::PRESS || getKeyState(KeyCode::W) == InputState::REPEAT) { pos += front * (dt * velocity.u); }
	if(getKeyState(KeyCode::S) == InputState::PRESS || getKeyState(KeyCode::S) == InputState::REPEAT) { pos -= front * (dt * velocity.u); }
	if(getKeyState(KeyCode::D) == InputState::PRESS || getKeyState(KeyCode::D) == InputState::REPEAT) { pos -= right * (dt * velocity.u); }
	if(getKeyState(KeyCode::A) == InputState::PRESS || getKeyState(KeyCode::A) == InputState::REPEAT) { pos += right * (dt * velocity.u); }



	delta = math::vec2f(getCursorDelta<f32>()) * velocity.v;
	if(delta.x != 0.0f && delta.y != 0.0f) 
	{
		rotation += delta * dt;
		rotation.pitch = std::clamp(rotation.pitch, -89.0f, 89.0f);
		rotRad = { math::radians(rotation.yaw  ), math::radians(rotation.pitch) };
		front = {
			cosf(rotRad.yaw) * cosf(rotRad.pitch),
			sinf(rotRad.pitch),
			sinf(rotRad.yaw) * cosf(rotRad.pitch)
		};
		front.normalize();
	}

	recalcView();
	return;
}