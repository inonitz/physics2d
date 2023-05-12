#include "raytrace.hpp"
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include <math.h>
#include "../context.hpp"
#include "../shader.hpp"
#include "../vertexArray.hpp"
#include "../texture.hpp"
#include "../random.hpp"




std::vector<f32> squareVertices =
{
	-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
	-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
};

std::vector<u32> squareIndices =
{
	0, 2, 1,
	0, 3, 2
};


math::vec3u recomputeDispatchSize(math::vec2u const& dims)
{
	math::vec3u localWorkgroupSize{32, 1, 1};
	i32 		max_group_invoc;
	math::vec2f tmp_cvt, tmp_cvt1;


	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_group_invoc);
	tmp_cvt.x = __scast(f32, max_group_invoc);       /* max_invoc */
	tmp_cvt.y = __scast(f32, localWorkgroupSize.x);  /* width of local workgroup, starting at 32 because most gpus will be ok with this as their starting subgroup size */
	tmp_cvt.x /= tmp_cvt.y; 				        /* height of local workgroup */
	tmp_cvt.x = std::ceil(tmp_cvt.x); 	    /* get ceiling of division */
	localWorkgroupSize.y = __scast( u32, tmp_cvt.x );


	tmp_cvt.x = __scast(f32, dims.x);
	tmp_cvt.y = __scast(f32, dims.y);
	tmp_cvt1.x = __scast(f32, localWorkgroupSize.x);
	tmp_cvt1.y = __scast(f32, localWorkgroupSize.y);
	tmp_cvt = tmp_cvt / tmp_cvt1;
	tmp_cvt.x = std::ceil(tmp_cvt.x);
	tmp_cvt.y = std::ceil(tmp_cvt.y);
	
	debug_messagefmt("recomputeDispatchSize:\n    Local workgroup Size of { %u %u %u }\n    Workgroup Dispatch Size { %u %u %u }\n", 
		localWorkgroupSize.x, localWorkgroupSize.y, 1,
		(u32)tmp_cvt.x, 	  (u32)tmp_cvt.y, 1
	);
	
	
	return {
		__scast(u32, tmp_cvt.x),
		__scast(u32, tmp_cvt.y),
		1u
	};
}


void renderImGui(i32& out_samples, f32 sample_randf, math::vec3u const& computeDispatchSize)
{
	auto* ctx = getGlobalContext();
	std::array<i32, 2> windowDims = ctx->glfw.dims;
	i32 work_grp_inf[7];
	f32 dt = ctx->glfw.time_dt();


	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0,  &work_grp_inf[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1,  &work_grp_inf[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2,  &work_grp_inf[2]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0,   &work_grp_inf[3]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1,   &work_grp_inf[4]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2,   &work_grp_inf[5]);
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inf[6]);
	
	
	ImGui::BeginGroup();
	ImGui::BulletText("Currently Using GPU Vendor %s Model %s\n", glad_glGetString(GL_VENDOR), glad_glGetString(GL_RENDERER));
	ImGui::BulletText("Max work-groups per compute shader: { %10d, %10d, %10d }\n", work_grp_inf[0], work_grp_inf[1], work_grp_inf[2]);
	ImGui::BulletText("Max work group size:                { %10d, %10d, %10d }\n", work_grp_inf[3], work_grp_inf[4], work_grp_inf[5]);
	ImGui::BulletText("Max invocations per work group: %d\n", work_grp_inf[6]);
	ImGui::BulletText("Compute Shader Invoked with %d * %d * %d (%d) Compute Groups\n", 
		computeDispatchSize.x, 
		computeDispatchSize.y, 
		1, 
		computeDispatchSize.x * computeDispatchSize.y
	);
	ImGui::EndGroup();
	ImGui::BeginGroup();
	ImGui::Text("Window Resolution is %ux%u\n", windowDims[0], windowDims[1]);
	ImGui::Text("Rendering at %.02f Frames Per Second (%.05f ms/frame)", (1.0f / dt), (dt * 1000.0f) );
	ImGui::Text("Random Float For Pixel Samples = %.05f               ", sample_randf);
	ImGui::SliderInt("Random Samples Per Pixel", &out_samples, 0, 128);
	ImGui::EndGroup();
	return;
}



int raytracer()
{
	auto* context = getGlobalContext();
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};
	bool running, focused = true, paused = false, changedResolution = false, refresh[2] = { false, false };
	u32 windowWidth = 1280, windowHeight = 720;
	i32 uniform_samplesppx = 16;
	f32 uniform_randnum    = randnorm32f();
	math::vec3u computeGroups = recomputeDispatchSize({ windowWidth, windowHeight });

	Program shader, compute;
	VertexArray   vao;
	TextureBuffer tex;
	Buffer vbo, ibo;
	// SceneData data = {
	// 	malloc(sizeof(Sphere) * 16),
	// 	16,
	// 	2,
	// 	CameraTransform{

	// 	}
	// }
	// ShaderStorageBuffer ssbo;
	
	

	/* Init */
    context->glfw.create(windowWidth, windowHeight, eventFuncs);	
	context->frameIndex = 0;
	

	/* Initialize OpenGL stuff */
	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.45f, 1.05f, 0.60f, 1.00f);





	shader.fromFiles({
		{ "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.vert", GL_VERTEX_SHADER   },
		{ "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.frag", GL_FRAGMENT_SHADER }
	});
	compute.fromFilesCompute({
			{ "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.comp", GL_COMPUTE_SHADER },
		},
		computeGroups
	);

	ifcrash(!shader.success() || !compute.success());


	vbo.create(BufferDescriptor{ 
			(void*)squareVertices.data(), 4u, 
			{{
				{ GL_FLOAT, 3 },
				{ GL_FLOAT, 2 }
			}}
		},
		GL_STREAM_DRAW
	);
	ibo.create(BufferDescriptor{ 
			(void*)squareIndices.data(), __scast(u32, squareIndices.size()), 
			{{
				{ GL_UNSIGNED_INT, 1 },
			}}
		},
		GL_STREAM_DRAW
	);
	vao.create(vbo, ibo);

	// ssbo.create(BufferDescriptor{
	// 	nullptr,

	// }, GL_DYNAMIC_STORAGE_BIT);


	tex.create(TextureBufferDescriptor
	{
		{ windowWidth, windowHeight },
		nullptr,
		{4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{
			{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  },
		}
	});
	tex.bindToImage(0, TEX_IMAGE_WRITE_ONLY_ACCESS);




	/* Main Loop */
	context->glfw.unlockCursor();
	running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
	while (running)
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        context->glfw.procUpcomingEvents();
		if(focused)
		{
			renderImGui(uniform_samplesppx, uniform_randnum, computeGroups);
			if(!paused)
			{
				/* Compute Shader Pass Here */
				compute.bind();
				compute.uniform1f("u_dt", uniform_randnum);
				compute.uniform1i("samples_per_pixel", uniform_samplesppx);
				glDispatchCompute(computeGroups.x, computeGroups.y, computeGroups.z);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				
				/* Vertex Shader Pass Here */
				shader.bind();
				tex.bindToUnit(0);
				shader.uniform1i("tex", 0);


				/* Bind VAO & Draw Here */
				vao.bind();
				glDrawElements(GL_TRIANGLES, squareIndices.size(), GL_UNSIGNED_INT, 0);
			}


			// context->glfw.setCursorMode( !paused);
            if(refresh[0]) {
                shader.reload();
                refresh[0] = shader.success();
            }
            if(refresh[1]) {
                compute.reload();
                refresh[1] = compute.success();
            }
			if(changedResolution)
			{
				windowWidth  = context->glfw.dims[0];
				windowHeight = context->glfw.dims[1];
				tex.recreateImage({ windowWidth, windowHeight });
				tex.bindToUnit(0);
				tex.bindToImage(0, TEX_IMAGE_WRITE_ONLY_ACCESS);
				
				computeGroups = recomputeDispatchSize({ windowWidth, windowHeight });
				compute.reloadCompute(computeGroups);
			}
		}


        running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
        focused = !context->glfw.minimized();
        paused  = paused ^ isKeyPressed(KeyCode::P);
		refresh[0] = isKeyPressed(KeyCode::NUM9);
        refresh[1] = isKeyPressed(KeyCode::NUM0);
		changedResolution = context->glfw.windowSizeChanged();


        context->glfw.procOngoingEvents();
		++context->frameIndex;
	}
	context->glfw.close();


	compute.destroy();
	shader.destroy();
	context->glfw.destroy();
	return 0;
}