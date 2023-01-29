#include "camera.hpp"
#include "callbackHooks.hpp"




void FPSCamera::create(
	math::vec4f const& perspectiveVector, /* fovy, aspect_ratio(w/h), near, far */	
	math::vec3f const& currentPos,
	math::vec3f const& frontVector,
	math::vec3f const& upVector,
	math::vec3f const& constVel
) {
	position    = currentPos;
	cameraFront = frontVector;
	cameraUp    = upVector;
	vel 	 = constVel;
	euler    = { 0.0f, 0.0f, 0.0f, 0.1f };
	projData = perspectiveVector;

	updateProjection();
	updateView();
	return;
}


void FPSCamera::updateFromKeyboard(double fixedDelta)
{
	static u8 controlStates = 0;
	controlStates  = (getKeyState(KeyCode::W) == InputState::PRESS);
	controlStates += (getKeyState(KeyCode::S) == InputState::PRESS) << 1;
	controlStates += (getKeyState(KeyCode::D) == InputState::PRESS) << 2;
	controlStates += (getKeyState(KeyCode::A) == InputState::PRESS) << 3;


	persp_changed = false;
	// markfmt("control states: %u {d=%u}\n", controlStates, getKeyState(KeyCode::D));
	if(likely(controlStates)) 
	{
		const math::vec3f speed = vel * static_cast<f32>(fixedDelta);
		if(controlStates & 0b0001) { position += cameraFront * speed; }
		if(controlStates & 0b0010) { position -= cameraFront * speed; }
		if(controlStates & 0b0100) { position += cameraRight * speed; }
		if(controlStates & 0b1000) { position -= cameraRight * speed; }
		markstr("Active Camera Position: ");
		position.print();
		persp_changed = true;
	}
	return;
}


void FPSCamera::updateFromMouse()
{
	std::array<f64, 2> cursorOffset = getCursorDelta();

	euler.yaw   += cursorOffset[0] * euler.w;
	euler.pitch += cursorOffset[1] * euler.w;
	euler.yaw   = std::clamp(euler.yaw  , -89.5f, 89.5f);
	euler.pitch = std::clamp(euler.pitch, -89.5f, 89.5f);

	updateBasisVectors();
	updateView();
}




void FPSCamera::updateBasisVectors()
{
	math::vec2f eulerRadians = { math::radians(euler.yaw), math::radians(euler.pitch) };
	cameraFront = {
		cosf(eulerRadians.yaw) * cosf(eulerRadians.pitch),
		sinf(eulerRadians.pitch),
		sinf(eulerRadians.yaw) * cosf(eulerRadians.pitch)
	};
	cameraFront.normalize();


	cameraRight = math::cross(cameraFront, worldUp    ); cameraRight.normalize();
	cameraUp    = math::cross(cameraFront, cameraRight); cameraUp.normalize();
	return; 
}


void FPSCamera::updateProjection()
{
	math::perspective(projData[0], projData[1], projData[2], projData[3],m_persp);
	math::inv_perspective(m_persp, m_inv_persp);
	persp_changed = false;
	return;
}


void FPSCamera::updateView()
{
	math::lookAt    (position, position + cameraFront, worldUp, m_view);
	// m_view.print();
	math::inv_lookAt(position, position + cameraFront, worldUp, m_inv_view);
	// m_inv_view.print();
	return;
}

