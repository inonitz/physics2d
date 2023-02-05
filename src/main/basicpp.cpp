#include "basicpp.hpp"
#include <stb_image/stb_image.hpp>
#include <ImGui/imgui.h>
#include <glad/glad.h>
#include "../context.hpp"
#include "../file.hpp"
#include "../vec.hpp"
#include <immintrin.h>
#include <stdexcept>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>




u32 reloadShaders(std::array<const char*, 2> const& paths);
u32 recreateImage(
	const char* path, 
	std::vector<std::pair<u32, u32> > const& texParameters
);
void convertRGBA_U32_TO_F32(u8* inData, float* outData, i32 dimx, i32 dimy);




int basicpp() {	
	auto* context = getGlobalContext();
	u32 			 windowWidth  = 960;
	u32 			 windowHeight = 480;
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};


	u32 shaderProgram;
	u32 VAO, VBO, EBO, TEX;
	std::array<float, 4> windowColor  		  = { 0.45f, 0.55f, 0.60f, 1.00f };
	std::array<float, 4> texelColorMultiplier = { 1.0f,   0.0f, 0.5f,   1.0f };

	// glm::mat4x4 model, view, proj;
	// glm::vec3 cameraPosition = { 0.0f, 0.0f, -3.0f };
	// glm::vec3 cameraFront    = { 0.0f, 0.0f,  1.0f };
	// glm::vec3 cameraUp       = { 0.0f, 1.0f,  0.0f };
	// glm::vec3 cameraRight, lookDir;
	f32 fieldOfView   = 90.0f;
	f32 aspectRatio   = windowWidth / (f32)windowHeight;
	f32 nearClipPlane = 0.1f;
	f32 farClipPlane  = 100.0f;
	
	math::vec2f aggregateAngels, delta;
	f32 cameraSpeed = 10.0f, cameraRotateSpeed = 0.04f;
	f32 dt;
	u8 skipFrames = 3;

	math::mat4f projection, viewMatrix, modelMatrix;
	math::vec3f camPos   = { 0.0f, 0.0f, -3.0f };
	math::vec3f camFront = { 0.0f, 0.0f,  1.0f };
	math::vec3f camUp    = { 0.0f, 1.0f,  0.0f }; 
	math::vec3f camRight;


	bool running = true, paused = false, focused = true;
	bool reloadShaderProgram = false;




	context->glfw.create(windowWidth, windowHeight, eventFuncs);
	// context->camera.create(
	// 	{ fieldOfView, aspectRatio, nearClipPlane, farClipPlane }, 
	// 	cameraPosition
	// );
	context->frameIndex = 0;


	/* Initialize OpenGL stuff */
	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(windowColor[0], windowColor[1], windowColor[2], windowColor[3]);


	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferStorage(VBO, vertices.size() * sizeof(f32), vertices.begin().base(), GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(EBO, indices.size()  * sizeof(u32), indices.begin().base(),  GL_DYNAMIC_STORAGE_BIT);


	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(VAO, EBO);

	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribFormat(VAO, 0, 4, GL_FLOAT, GL_FALSE, 0              );
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32));
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	

	TEX = recreateImage(assetPaths[3], 
		{
			{ GL_TEXTURE_WRAP_S, 	   GL_REPEAT 			},
			{ GL_TEXTURE_WRAP_T, 	   GL_REPEAT 			},
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 			    } 
		});
	
	shaderProgram = reloadShaders({ assetPaths[0], assetPaths[1] });


	u32 uniformColorLocation = glGetUniformLocation(shaderProgram, "texelMultiply");
	u32 uniformModelMatrix   = glGetUniformLocation(shaderProgram, "model");
	u32 uniformViewMatrix    = glGetUniformLocation(shaderProgram, "view");
	u32 uniformProjMatrix    = glGetUniformLocation(shaderProgram, "projection");
	u32 uniformTexSampler    = glGetUniformLocation(shaderProgram, "texData");

	
	glBindVertexArray(VAO);
	glUseProgram(shaderProgram);
	glBindTextureUnit(1, TEX);
	glUniform1i(uniformTexSampler, 1);

	math::mat4f* print;	


	auto ImGuiRender = [&]() {
		/*
			ImGui Input Capture & rendering
		*/
		ImGui::BeginGroup();
		ImGui::SliderFloat4("Texel Color Multiplier", texelColorMultiplier.begin(), 0.0f, 1.0f);
		ImGui::SliderFloat("Field Of View      ", &fieldOfView, 20.0f, 200.0f);
		ImGui::SliderFloat("Near Clip Plane    ", &nearClipPlane, 0.01f, 50.0f);
		ImGui::SliderFloat("Far  Clip Plane    ", &farClipPlane, 50.0f, 1000.0f);
		ImGui::SliderFloat("Camera Speed       ", &cameraSpeed, 1.0f, 15.0f);
		ImGui::SliderFloat("Camera Rotate Speed", &cameraRotateSpeed, 0.0001, 1.0f);
		aspectRatio = context->glfw.aspectRatio<f32>();
		ImGui::EndGroup();


		ImGui::BeginGroup();
		// print = reinterpret_cast<math::mat4f*>(glm::value_ptr(proj));
		print = &projection;
		ImGui::Text("Rendering at %.02f Frames Per Second (%.05f ms/frame)", (1.0f / dt), (dt * 1000.0f) );
		ImGui::Text("Proj:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
			print->m00, print->m01, print->m02, print->m03,
			print->m10, print->m11, print->m12, print->m13,
			print->m20, print->m21, print->m22, print->m23,
			print->m30, print->m31, print->m32, print->m33
		);
		// print = reinterpret_cast<math::mat4f*>(glm::value_ptr(view));
		print = &viewMatrix;
		ImGui::Text("View:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
			print->m00, print->m01, print->m02, print->m03,
			print->m10, print->m11, print->m12, print->m13,
			print->m20, print->m21, print->m22, print->m23,
			print->m30, print->m31, print->m32, print->m33
		);
		// print = reinterpret_cast<math::mat4f*>(glm::value_ptr(model));
		print = &modelMatrix;
		ImGui::Text("Model:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
			print->m00, print->m01, print->m02, print->m03,
			print->m10, print->m11, print->m12, print->m13,
			print->m20, print->m21, print->m22, print->m23,
			print->m30, print->m31, print->m32, print->m33
		);
		ImGui::EndGroup();
	};


	
	auto gameLoop = [&]() 
	{
		/*
			update shader if appropriate key pressed.
		*/
		if(reloadShaderProgram)
		{
			u32 tmp = reloadShaders({ assetPaths[0], assetPaths[1] });
			if(tmp != DEFAULT32)
			{
				reloadShaderProgram = false;

				debug_message("Reloading Shaders!...\n");
				glDeleteProgram(shaderProgram);
				
				shaderProgram = tmp;
				glUseProgram(shaderProgram);

				uniformColorLocation = glGetUniformLocation(shaderProgram, "texelMultiply");
				uniformModelMatrix   = glGetUniformLocation(shaderProgram, "model");
				uniformViewMatrix    = glGetUniformLocation(shaderProgram, "view");
				uniformProjMatrix    = glGetUniformLocation(shaderProgram, "projection");
				uniformTexSampler    = glGetUniformLocation(shaderProgram, "texData");
				glUniform1i(uniformTexSampler, 1); /* this can stay here for now, we're not doing anything fancy with textures/models. */
			}
		}

		
		/*
			Process Movement-Related Inputs captured by glfw (for Camera Movement)
		*/
		// cameraRight = glm::normalize(glm::cross(cameraUp, cameraFront));
		// if(getKeyState(KeyCode::W) == InputState::PRESS || getKeyState(KeyCode::W) == InputState::REPEAT) { cameraPosition += cameraFront * dt * cameraSpeed; }
		// if(getKeyState(KeyCode::S) == InputState::PRESS || getKeyState(KeyCode::S) == InputState::REPEAT) { cameraPosition -= cameraFront * dt * cameraSpeed; }
		// if(getKeyState(KeyCode::D) == InputState::PRESS || getKeyState(KeyCode::D) == InputState::REPEAT) { cameraPosition += cameraRight * dt * cameraSpeed; }
		// if(getKeyState(KeyCode::A) == InputState::PRESS || getKeyState(KeyCode::A) == InputState::REPEAT) { cameraPosition -= cameraRight * dt * cameraSpeed; }
		/* because camRight is actually point to -x axis in a right-handed coordinate system, we flip the sign's when adding to camPos. */
		camRight = math::cross(camUp, camFront); camRight.normalize();
		if(getKeyState(KeyCode::W) == InputState::PRESS || getKeyState(KeyCode::W) == InputState::REPEAT) { camPos += camFront * (dt * cameraSpeed); }
		if(getKeyState(KeyCode::S) == InputState::PRESS || getKeyState(KeyCode::S) == InputState::REPEAT) { camPos -= camFront * (dt * cameraSpeed); }
		if(getKeyState(KeyCode::D) == InputState::PRESS || getKeyState(KeyCode::D) == InputState::REPEAT) { camPos -= camRight * (dt * cameraSpeed); }
		if(getKeyState(KeyCode::A) == InputState::PRESS || getKeyState(KeyCode::A) == InputState::REPEAT) { camPos += camRight * (dt * cameraSpeed); }


		delta = getCursorDelta<f32>();
		delta *= cameraRotateSpeed;
		if(delta.x != 0.0f && delta.y != 0.0f) {
			aggregateAngels += delta;
			aggregateAngels.pitch = std::clamp(aggregateAngels.pitch, -89.0f, 89.0f);
			
			math::vec2f eulerRadians = { 
				math::radians(aggregateAngels.yaw  ), 
				math::radians(aggregateAngels.pitch) 
			};
			// cameraFront = glm::normalize(glm::vec3(
			// {
			// 	cosf(eulerRadians.yaw) * cosf(eulerRadians.pitch),
			// 	sinf(eulerRadians.pitch),
			// 	sinf(eulerRadians.yaw) * cosf(eulerRadians.pitch)
			// }));
			camFront = {
				cosf(eulerRadians.yaw) * cosf(eulerRadians.pitch),
				sinf(eulerRadians.pitch),
				sinf(eulerRadians.yaw) * cosf(eulerRadians.pitch)
			};
			camFront.normalize();
		}


		
		/*
			Update OpenGL Uniforms
		*/
		// lookDir = cameraPosition + cameraFront;
		// model = glm::identity<glm::mat4x4>();
		// view  = glm::lookAtRH(cameraPosition, cameraPosition + cameraFront, cameraUp);
		// proj  = glm::perspective(fieldOfView, aspectRatio, nearClipPlane, farClipPlane);

		// memcpy(camPos.begin()  , glm::value_ptr(cameraPosition), camPos.bytes());
		// memcpy(camUp.begin()   , glm::value_ptr(cameraUp	  ), camUp.bytes()   );
		// memcpy(camFront.begin(), glm::value_ptr(lookDir	  ), camFront.bytes());
		math::identity(modelMatrix);
		math::lookAt(camPos, camPos + camFront, camUp,viewMatrix);
		math::perspective(aspectRatio, math::radians(fieldOfView), nearClipPlane, farClipPlane, projection);
		

		glUniform4f(
			uniformColorLocation,  
			texelColorMultiplier[0], 
			texelColorMultiplier[1], 
			texelColorMultiplier[2], 
			texelColorMultiplier[3]
		);
		glUniformMatrix4fv(uniformModelMatrix, 1, false, modelMatrix.begin());
		glUniformMatrix4fv(uniformViewMatrix,  1, false, viewMatrix.begin());
		glUniformMatrix4fv(uniformProjMatrix,  1, false, projection.begin());
		// glm::value_ptr(proj) - GOOD
		// glm::value_ptr(view) - GOOD
		// glm::value_ptr(model) - GOOD

		/*
			Render.
		*/
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	};



	while(running)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		context->glfw.procUpcomingEvents();
		
		
		if(focused) 
		{		
			ImGuiRender();

			if(!paused && context->frameIndex > skipFrames) {
				context->glfw.lockCursor();
				gameLoop();
			} else {
				context->glfw.unlockCursor();
			}
			

			dt = context->glfw.time_dt();
		}
		

		reloadShaderProgram = boolean(getKeyState(KeyCode::R) == InputState::PRESS);
		running 			= boolean(getKeyState(KeyCode::ESCAPE) != InputState::PRESS) && !context->glfw.shouldClose();
		focused 			= !context->glfw.minimized();
		if(getKeyState(KeyCode::P) == InputState::PRESS) {
			paused = !paused;
		}

		
		context->glfw.procOngoingEvents();
		++context->frameIndex;
		
	}
	context->glfw.close();

	// while(!context->glfw.shouldClose())
	// {
	// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 	context->glfw.procUpcomingEvents();

	// 	if(unlikely(captureInputState[5] == InputState::PRESS))
	// 		context->glfw.close();

	// 	if(likely( !context->glfw.minimized() && context->frameIndex > skipFrames && !pause) ) 
	// 	{
	// 		ImGui::SliderFloat4("Texel Color Multiplier", texelColorMultiplier.begin(), 0.0f, 1.0f);
	// 		ImGui::BeginGroup();
	// 		ImGui::SliderFloat("Field Of View      ", &fieldOfView, 20.0f, 200.0f);
	// 		ImGui::SliderFloat("Near Clip Plane    ", &nearClipPlane, 0.01f, 50.0f);
	// 		ImGui::SliderFloat("Far  Clip Plane    ", &farClipPlane, 50.0f, 1000.0f);
	// 		ImGui::SliderFloat("Camera Speed       ", &cameraSpeed, 1.0f, 15.0f);
	// 		ImGui::SliderFloat("Camera Rotate Speed", &cameraRotateSpeed, 0.1, 3.0f);
	// 		aspectRatio = context->glfw.aspectRatio<f32>();
	// 		ImGui::EndGroup();
	// 		ImGui::BeginGroup();
	// 		print = reinterpret_cast<math::mat4f*>(glm::value_ptr(proj));
	// 		ImGui::Text("Proj:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
	// 			print->m00, print->m01, print->m02, print->m03,
	// 			print->m10, print->m11, print->m12, print->m13,
	// 			print->m20, print->m21, print->m22, print->m23,
	// 			print->m30, print->m31, print->m32, print->m33
	// 		);

	// 		print = reinterpret_cast<math::mat4f*>(glm::value_ptr(view));
	// 		ImGui::Text("View:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
	// 			print->m00, print->m01, print->m02, print->m03,
	// 			print->m10, print->m11, print->m12, print->m13,
	// 			print->m20, print->m21, print->m22, print->m23,
	// 			print->m30, print->m31, print->m32, print->m33
	// 		);
	// 		// ImGui::Text(
	// 		// 	"Position:   ( %+.05f, %+.05f, %+.05f )\nMouseDelta: ( %+.05f, %+.05f )\nCam Front:  ( %+.05f, %+.05f, %+.05f )\n",
	// 		// 	p.x, p.y, p.z, 
	// 		// 	d.x, d.y,
	// 		// 	f.x, f.y, f.z
	// 		// );

	// 		print = reinterpret_cast<math::mat4f*>(glm::value_ptr(model));
	// 		ImGui::Text("Model:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
	// 			print->m00, print->m01, print->m02, print->m03,
	// 			print->m10, print->m11, print->m12, print->m13,
	// 			print->m20, print->m21, print->m22, print->m23,
	// 			print->m30, print->m31, print->m32, print->m33
	// 		);
	// 		ImGui::EndGroup();


	// 		captureInputState = {
	// 			getKeyState(KeyCode::W),
	// 			getKeyState(KeyCode::S),
	// 			getKeyState(KeyCode::D),
	// 			getKeyState(KeyCode::A),
	// 			getKeyState(KeyCode::R),
	// 			getKeyState(KeyCode::Q),
	// 			getMouseButtonState(MouseButton::LEFT),
	// 			getMouseButtonState(MouseButton::RIGHT)
	// 		};
	// 		dt = context->glfw.time_dt();
	// 		lockCursorToScreen = boolean((u8)captureInputState[7] - 1); /* PRESS = 2 , RELEASE = 1. ==> if PRESS, unlock cursor */


	// 		if(captureInputState[4] == InputState::PRESS) {
	// 			u32 tmp = reloadShaders({ assetPaths[0], assetPaths[1] });
	// 			if(tmp != DEFAULT32) {
	// 				debug_message("Reloading Shaders!...\n");
	// 				glDeleteProgram(shaderProgram);
	// 				shaderProgram = tmp;
	// 				glUseProgram(shaderProgram);

	// 				uniformColorLocation = glGetUniformLocation(shaderProgram, "texelMultiply");
	// 				uniformModelMatrix   = glGetUniformLocation(shaderProgram, "model");
	// 				uniformViewMatrix    = glGetUniformLocation(shaderProgram, "view");
	// 				uniformProjMatrix    = glGetUniformLocation(shaderProgram, "projection");
	// 				uniformTexSampler    = glGetUniformLocation(shaderProgram, "texData");

	// 				glUniform1i(uniformTexSampler, 1);
	// 			}
	// 		}


	// 		cameraRight = glm::normalize(glm::cross(cameraUp, cameraFront)); 
	// 		if(captureInputState[0] == InputState::PRESS || captureInputState[0] == InputState::REPEAT) {
	// 			cameraPosition += cameraFront * dt * cameraSpeed;
	// 		}
	// 		if(captureInputState[1] == InputState::PRESS || captureInputState[1] == InputState::REPEAT) {
	// 			cameraPosition -= cameraFront * dt * cameraSpeed;
	// 		}
	// 		if(captureInputState[2] == InputState::PRESS || captureInputState[2] == InputState::REPEAT) {
	// 			cameraPosition += cameraRight * dt * cameraSpeed;
	// 		}
	// 		if(captureInputState[3] == InputState::PRESS || captureInputState[3] == InputState::REPEAT) {
	// 			cameraPosition -= cameraRight * dt * cameraSpeed;
	// 		}


	// 		delta = getCursorDelta<f32>();
	// 		delta *= cameraRotateSpeed;
	// 		if(delta.x != 0.0f && delta.y != 0.0f) {
	// 			aggregateAngels += delta;
	// 			aggregateAngels.pitch = std::clamp(aggregateAngels.pitch, -89.0f, 89.0f);
				
	// 			math::vec2f eulerRadians = { 
	// 				math::radians(aggregateAngels.yaw  ), 
	// 				math::radians(aggregateAngels.pitch) 
	// 			};
	// 			cameraFront = glm::normalize(glm::vec3(
	// 			{
	// 				cosf(eulerRadians.yaw) * cosf(eulerRadians.pitch),
	// 				sinf(eulerRadians.pitch),
	// 				sinf(eulerRadians.yaw) * cosf(eulerRadians.pitch)
	// 			}));
	// 		}


	// 		model = glm::identity<glm::mat4x4>();
	// 		view  = glm::lookAtRH(cameraPosition, cameraPosition + cameraFront, cameraUp);
	// 		proj  = glm::perspective(fieldOfView, aspectRatio, nearClipPlane, farClipPlane);


	// 		glUniform4f(
	// 			uniformColorLocation,  
	// 			texelColorMultiplier[0], 
	// 			texelColorMultiplier[1], 
	// 			texelColorMultiplier[2], 
	// 			texelColorMultiplier[3]
	// 		);
	// 		glUniformMatrix4fv(uniformModelMatrix, 1, false, glm::value_ptr(model));
	// 		glUniformMatrix4fv(uniformViewMatrix,  1, false, glm::value_ptr(view));
	// 		glUniformMatrix4fv(uniformProjMatrix,  1, false, glm::value_ptr(proj));
			
			
	// 		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	// 	}
		

	// 	if(getKeyState(KeyCode::ESCAPE) == InputState::PRESS) {
	// 		pause = !pause;
	// 		lockCursorToScreen = !lockCursorToScreen;
	// 	}
	// 	if(lockCursorToScreen)
	// 		context->glfw.lockCursor();

	// 	context->glfw.procOngoingEvents();
	// 	++context->frameIndex;
	// }

	
	glDeleteProgram(shaderProgram);
	glDeleteTextures(1, &TEX);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	context->glfw.destroy();
	return 0;
}




u32 reloadShaders(std::array<const char*, 2> const& paths) 
{
	static char genericErrorLog[1024];
	size_t size   		  = 0;
	char*  src    		  = nullptr;
	i32    length 		  = 0;
	i32    successStatus  = GL_TRUE;
	std::array<u32, 2> shaders;
	u32    			   shaderTypes[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	u32    			   newProgram = glCreateProgram();


	for(u32  i = 0; i < shaders.size() && successStatus; ++i)
	{
		shaders[i] = glCreateShader(shaderTypes[i]);

		size = 0;
		src  = nullptr;

		loadFile(paths[i], &size, src);
		src    = amalloc_t(char, size, CACHE_LINE_BYTES);
		loadFile(paths[i], &size, src);
		length = static_cast<i32>(size);


		glShaderSource(shaders[i], 1, (const char**)&src, &length);
		glCompileShader(shaders[i]);
		glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &successStatus);
		if(successStatus) {
			glAttachShader(newProgram, shaders[i]);
		} else {
			glGetShaderInfoLog(shaders[i], sizeof(genericErrorLog), &length, genericErrorLog);
        	printf("Shader [type %u] Error Log[%u Bytes]: %s\n", shaderTypes[i], length, genericErrorLog);
		}
		// ifcrashdo(!successStatus, { /* Safety Check. More info in debug mode. */
		// 	glGetShaderInfoLog(shaders[i], sizeof(genericErrorLog), &length, genericErrorLog);
        // 	printf("Shader [type %u] Error Log[%u Bytes]: %s\n", shaderTypes[i], length, genericErrorLog);
    	// });
		afree_t(src);
	}

	if(!successStatus) {
		for(auto& shader: shaders) { glDeleteShader(shader); }
		glDeleteProgram(newProgram);
		return DEFAULT32;
	}


	glLinkProgram(newProgram);
	glGetProgramiv(newProgram, GL_LINK_STATUS, &successStatus);
	ifcrashdo(!successStatus, {
		glGetProgramInfoLog(newProgram, sizeof(genericErrorLog), NULL, genericErrorLog);
		printf("%s\n", genericErrorLog);
	});


	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);
	return newProgram;
}



/*
	Assume the texture loaded is GL_TEXTURE_2D, With GL_RGBA32F as internal type
*/
u32 recreateImage(
	const char* path, 
	std::vector<std::pair<u32, u32> > const& texParameters
) {
	u32 TEX;
	i32 TEXW, TEXH, channels, desiredChannels = 4;
	u8* textureBuffer;
	f32* textureBufferAsFloat;


	stbi_set_flip_vertically_on_load(true);
	textureBuffer 		 = stbi_load(path, &TEXW, &TEXH, &channels, desiredChannels);
	textureBufferAsFloat = amalloc_t(f32, (size_t)TEXW * TEXH * sizeof(f32) * desiredChannels, CACHE_LINE_BYTES);
	convertRGBA_U32_TO_F32(textureBuffer, textureBufferAsFloat, TEXW, TEXH);
	stbi_image_free(textureBuffer);


	glCreateTextures(GL_TEXTURE_2D, 1, &TEX);
	glBindTexture(GL_TEXTURE_2D, TEX);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		TEXW,
		TEXH,
		0,
		GL_RGBA,
		GL_FLOAT,
		textureBufferAsFloat	
	);
	afree_t(textureBufferAsFloat);
	glBindTexture(GL_TEXTURE_2D, 0);


	for(auto& texParam : texParameters) {
		glTextureParameteri(TEX, texParam.first, texParam.second);	
	}
	glGenerateTextureMipmap(TEX);
	
	
	return TEX;
}




void convertRGBA_U32_TO_F32(u8* inData, float* outData, i32 dimx, i32 dimy)
{
    size_t currPixel = 0;
    size_t bufLength = 4 * static_cast<size_t>(dimx) * dimy;

#ifdef __AVX2__
    markstr("AVX2\n");
    ifcrashdo( ( (size_t)outData & (8 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 32 Byte memory boundary\n");
    });
	__m256  constf32;
	__m128i A; 
	__m256i B;
	__m256  C;
	__m256  D;
	constf32 = _mm256_broadcastss_ps(_mm_set_ss(0.00392156862745098f)); /* load constant 1/255 to each element */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) 
	// 	{
	// 		for(i32 x = 0; x < dimx; x += 8) 
	// 		{
	// 			A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
	// 			B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
	// 			C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
	// 			D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
	// 		    _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
	// 			currPixel += 8;
	// 		}
	// 	}  
	// }
    for(currPixel = 0; currPixel < bufLength; currPixel += 8) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
        B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
        C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
        D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
        _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
    }
#elif defined __SSE4_1__
    markstr("SSE4.1\n");
    ifcrashdo( ( (size_t)outData & (4 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 16 Byte memory boundary\n");
    });
    alignsz(16) std::array<f32, 4> const128 = { 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f };
    __m128 cf32;
    __m128i A, B;
	__m128  C, D;


    cf32 = _mm_load_ps(const128.begin()); /* SSE */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) 
	// 		{
    //             A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
    //             B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
    //             C = _mm_cvtepi32_ps(B);   /* SSE2   */
    //             D = _mm_mul_ps(C, cf32);  /* SSE    */
    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
        B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
        C = _mm_cvtepi32_ps(B);   /* SSE2   */
        D = _mm_mul_ps(C, cf32);  /* SSE    */
        _mm_store_ps(&outData[currPixel], D);
    }
#else
    markstr("FALLBACK (COMPILER_OPTIMIZE march=native, mtune=native\n");
    alignsz(16) std::array<f32, 4> tmpf{};
    alignsz(16) std::array<u32, 4> tmpu{};
    u32   tmp = 0;


    // for(i32 c = 0; c < 4; ++c) 
	// {   
    //     for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) {
    //             memcpy(&tmp, &inData[currPixel], 4);
    //             tmpu = {
    //                 tmp & 0x000000FF, /* First  Byte */
    //                 tmp & 0x0000FF00, /* Second Byte */
    //                 tmp & 0x00FF0000, /* Third  Byte */
    //                 tmp & 0xFF000000  /* Fourth Byte */
    //             };
    //             tmpf = {
    //                 static_cast<f32>(tmpu[0]),
    //                 static_cast<f32>(tmpu[1]),
    //                 static_cast<f32>(tmpu[2]),
    //                 static_cast<f32>(tmpu[3]),
    //             };
    //             memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);

    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) 
    {
        memcpy(&tmp, &inData[currPixel], 4);
        tmpu = {
            tmp & 0x000000FF, /* First  Byte */
            tmp & 0x0000FF00, /* Second Byte */
            tmp & 0x00FF0000, /* Third  Byte */
            tmp & 0xFF000000  /* Fourth Byte */
        };
        tmpf = {
            static_cast<f32>(tmpu[0]),
            static_cast<f32>(tmpu[1]),
            static_cast<f32>(tmpu[2]),
            static_cast<f32>(tmpu[3]),
        };
        memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);
    }
#endif

    return;
}



/* 
	Current Goals:
	create a camera struct that handles the camera-relevant data & updates. => TEST AFTER WRITING!

	then try =>
		rendering the grid, with each cube being referencing its own 'particle data'
		compute shader


	later goals:
		render cube
		expand grid to 3D
		instanced rendering? (a lot of draw calls is no good)
*/





