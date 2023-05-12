#include "basicpp.hpp"
#include <glad/glad.h>
#include <stb_image/stb_image.hpp>
#include <ImGui/imgui.h>
#include "../context.hpp"
#include "../shader.hpp"




void renderImGui(globalContext* const ctx, math::mat4f const& modelMatrix)
{
	const math::mat4f* print = nullptr;
	const f32          dt 	 = ctx->glfw.time_dt();

	/*
		ImGui Input Capture & rendering
	*/
	ImGui::BeginGroup();
	
	// ImGui::BeginGroup();
	// ImGui::SliderInt("WorkGroup [x]", &workGroupSizeTest[0], 1, windowWidth);
	// ImGui::SliderInt("WorkGroup [y]", &workGroupSizeTest[1], 1, windowHeight);
	// ImGui::EndGroup();

	ImGui::SliderFloat("Field Of View      ", &ctx->persp.__.fieldOfView, 20.0f, 200.0f);
	ImGui::SliderFloat("Near Clip Plane    ", &ctx->persp.__.nearClip, 0.01f, 50.0f);
	ImGui::SliderFloat("Far  Clip Plane    ", &ctx->persp.__.farClip, 50.0f, 1000.0f);
	ImGui::SliderFloat("Camera Speed       ", &ctx->cam.velocity.u, 1.0f, 50.0f);
	ImGui::SliderFloat("Camera Rotate Speed", &ctx->cam.velocity.v, 0.01, 7.5f);
	// ctx->persp.__.aspectRatio = ctx->glfw.aspectRatio<f32>();
	ImGui::EndGroup();


	ImGui::BeginGroup();
	print = ctx->persp.constptr();
	ImGui::Text("Rendering at %.02f Frames Per Second (%.05f ms/frame)", (1.0f / dt), (dt * 1000.0f) );
	ImGui::Text("Proj:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
		print->m00, print->m01, print->m02, print->m03,
		print->m10, print->m11, print->m12, print->m13,
		print->m20, print->m21, print->m22, print->m23,
		print->m30, print->m31, print->m32, print->m33
	);
	print = ctx->cam.constptr();
	ImGui::Text("View:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
		print->m00, print->m01, print->m02, print->m03,
		print->m10, print->m11, print->m12, print->m13,
		print->m20, print->m21, print->m22, print->m23,
		print->m30, print->m31, print->m32, print->m33
	);
	print = &modelMatrix;
	ImGui::Text("Model:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
		print->m00, print->m01, print->m02, print->m03,
		print->m10, print->m11, print->m12, print->m13,
		print->m20, print->m21, print->m22, print->m23,
		print->m30, print->m31, print->m32, print->m33
	);
	ImGui::EndGroup();
}




int basicpp()
{
	auto* context = getGlobalContext();
	u32 			 windowWidth  = 1280;
	u32 			 windowHeight = 720;
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};
	

	Program shader, compute;
	u32 VAO, VBO, EBO;
	math::mat4f modelMatrix; 
    math::identity(modelMatrix);
	
    
    context->glfw.create(windowWidth, windowHeight, eventFuncs);
	context->cam.create({ 0.0f, 0.0f, -3.0f });
	context->persp.create({ windowWidth / (f32)windowHeight, 90.0f, 0.1f, 100.0f });
	context->frameIndex = 0;
	


	shader.fromFiles({
		{ assetPaths[0], GL_VERTEX_SHADER   },
		{ assetPaths[1], GL_FRAGMENT_SHADER }
	});
	compute.fromFiles({
		{ assetPaths[2], GL_COMPUTE_SHADER }
	});


	/* Initialize OpenGL stuff */
	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);


	/* Create VAO, VBO, EBO */
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);
    glNamedBufferData(VBO, vertices.size() * sizeof(f32), vertices.data(), GL_STATIC_DRAW);
    glNamedBufferData(EBO,  indices.size() * sizeof(u32),  indices.data(), GL_STATIC_DRAW);

	glEnableVertexArrayAttrib (VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribFormat (VAO, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glEnableVertexArrayAttrib (VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat (VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex));
    
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(VAO, EBO);


    u32 texture;
    i32 width = 1024, height = 1024;
    // i32 channels;
    // f32* buffer;
    // buffer = stbi_loadf(assetPaths[3], &width, &height, &channels, 4);

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, 	GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, 	GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(texture, 1, GL_RGBA32F, width, height);
    // glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F); 


    bool running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
    bool focused = true;
    bool paused  = false;
    bool refresh[2] = { false, false };
    while (running)
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        context->glfw.procUpcomingEvents();
		if(focused)
		{
			renderImGui(context, modelMatrix);
			if(!paused)
			{
				context->cam.onUpdate(context->glfw.time_dt());
				context->persp.recalculate();
			
				compute.bind();
				glDispatchCompute(width / 64, height, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				shader.bind();
				glBindTextureUnit(0, texture);
				shader.uniform1i("texData", 0);
				shader.uniformMatrix4fv("model", modelMatrix);
				shader.uniformMatrix4fv("view", context->cam.constref());
				shader.uniformMatrix4fv("projection", context->persp.constref());
				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
			}

			context->glfw.setCursorMode( !paused);
            if(refresh[0]) {
                shader.reload();
                refresh[0] = shader.success();
            }
            if(refresh[1]) {
                compute.reload();
                refresh[1] = compute.success();
            }
		}


        running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
        focused = !context->glfw.minimized();
        paused  = paused ^ isKeyPressed(KeyCode::P);
        refresh[0] = isKeyPressed(KeyCode::NUM9);
        refresh[1] = isKeyPressed(KeyCode::NUM0);


        context->glfw.procOngoingEvents();
		++context->frameIndex;
	}
	context->glfw.close();


	compute.destroy();
	shader.destroy();
    glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	context->glfw.destroy();
	return 0;
}