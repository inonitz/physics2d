#include "raytrace.hpp"
#include "gl/shader2.hpp"
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"




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
	bool running, focused = true, paused = false, changedResolution = false, refresh[3] = { false, false, false };
	u8  sphereCount = 13;
	u32 windowWidth = 1280, windowHeight = 720;
	i32 uniform_samplesppx    = 50;
	i32 uniform_diffRecursion = 10;
	f32 camViewportWidth;
	f32 dt;
	math::vec4f uniform_randnum = { randnorm32f(), randnorm32f(), randnorm32f(), randnorm32f() };
	std::array< std::pair<const char*, u32>, 5> shaderStrings;
	std::array< std::pair<char*,       u32>, 3> fullShaderPaths;

	u8 shaderPathPrependIdx = 1;
	shaderStrings = {
		std::make_pair("C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/", 54),
		std::make_pair("C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/", 85),
		std::make_pair("shader.vert", 12),
		std::make_pair("shader.frag", 12),
		std::make_pair("shader_diffuse.comp", 20)
	};
	fullShaderPaths[0] = std::make_pair(__scast(char*, malloc(300)), 100);
	fullShaderPaths[1] = std::make_pair(fullShaderPaths[0].first + 100, 100);
	fullShaderPaths[2] = std::make_pair(fullShaderPaths[0].first + 200, 100);
	for(size_t i = 0; i < 3; ++i) { 
		// .first  = buffer ptr .second = buffer size
		memcpy(fullShaderPaths[i].first,                                              shaderStrings[shaderPathPrependIdx].first, shaderStrings[shaderPathPrependIdx].second);
		memcpy(fullShaderPaths[i].first + shaderStrings[shaderPathPrependIdx].second, shaderStrings[2 + i].first, 				shaderStrings[2 + i].second				  );
	}
	debug_messagefmt("Shader paths are: \n%s\n%s\n%s\n", 
		fullShaderPaths[0].first, 
		fullShaderPaths[1].first, 
		fullShaderPaths[2].first
	);


	ComputeGroupSizes invocDims;
	Program shader, compute;
	VertexArray   vao;
	TextureBuffer tex;
	Buffer vbo, ibo;
	SceneData* sceneDescription = nullptr;
	ShaderStorageBuffer ssbo;

	sceneDescription = __scast(SceneData*, malloc( sizeof(SceneData) + (sphereCount - 1) * sizeof(Sphere) ));
	sceneDescription->transform = {
		math::vec3f{ 0.0f, 0.0f, 0.0f },         					  /* Position 				*/
		math::vec2f{ context->glfw.aspectRatio<f32>() * 2.0f, 2.0f }, /* Viewport Width, height */
		1.0f,                           							  /* focal length 			*/
		0                               							  /* reserved 				*/
	};
	sceneDescription->curr_size = 2;
	sceneDescription->max_size  = 13;
	sceneDescription->objects[0] = Sphere{ {0.0f,    0.0f, -1.0f, 0.5f  } };
	sceneDescription->objects[1] = Sphere{ {0.0f, -100.5f, -1.0f, 100.0f} };
	



	/* Init */
    context->glfw.create(windowWidth, windowHeight, eventFuncs);	
	context->frameIndex = 0;


	debug( /* Enable Advanced OpenGL Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.45f, 1.05f, 0.60f, 1.00f);


	shader.createFrom({{ fullShaderPaths[0].first, GL_VERTEX_SHADER   },
					   { fullShaderPaths[1].first, GL_FRAGMENT_SHADER }
	});
	compute.createFrom({{ fullShaderPaths[2].first, GL_COMPUTE_SHADER }}); 


	invocDims = recomputeDispatchSize({ windowWidth, windowHeight });
	compute.resizeLocalWorkGroup(0, invocDims.localGroup);
	ifcrashdo(shader.compile()  == GL_FALSE, debug_message("Problem Compiling Vertex-Fragment Shader\n"));
	ifcrashdo(compute.compile() == GL_FALSE, debug_message("Problem Compiling Compute Shader\n")		);


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


	sceneDescription->transform.viewport.x = context->glfw.aspectRatio<f32>() * 2.0f;
	ssbo.create(BufferDescriptor{
			sceneDescription,
			__scast(u32, sizeof(SceneData) + (sphereCount - 1) * sizeof(Sphere) ),
			{{
				{ GL_UNSIGNED_BYTE, 1 }
			}}
		},
		GL_DYNAMIC_DRAW
	);
	ssbo.setBindingIndex(1);
	ssbo.bind();



	/* Main Loop */
	context->glfw.unlockCursor();
	running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
	while (running)
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        context->glfw.procUpcomingEvents();
		if(focused)
		{
			renderImGui(
				sceneDescription,
				dt,
				uniform_samplesppx, 
				uniform_diffRecursion, 
				uniform_randnum,
				invocDims.dispatchGroup
			);
			if(!paused)
			{
				/* Compute Shader Pass Here */
				compute.bind();
				ssbo.bind();
				compute.uniform1f("u_dt", dt);
				compute.uniform4fv("u_rand"         , uniform_randnum.begin());
				compute.uniform1i("u_samplesPpx"  , uniform_samplesppx     );
				compute.uniform1i("u_recurseDepth", uniform_diffRecursion  );
				glDispatchCompute(
					invocDims.dispatchGroup.x, 
					invocDims.dispatchGroup.y, 
					invocDims.dispatchGroup.z
				);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				
				/* Vertex Shader Pass Here */
				shader.bind();
				tex.bindToUnit(0);
				shader.uniform1i("tex", 0);


				/* Bind VAO & Draw Here */
				vao.bind();
				glDrawElements(GL_TRIANGLES, squareIndices.size(), GL_UNSIGNED_INT, 0);
			}


			//context->glfw.setCursorMode( !paused);
            if(refresh[0]) {
                shader.refreshFromFiles();
                refresh[0] = !shader.compile();
            }
			if(refresh[1]) {
				compute.refreshFromFiles();
				compute.resizeLocalWorkGroup(0, invocDims.localGroup);
			}
            if(refresh[2]) {
				refresh[2] = !compute.compile();
            }


			if(changedResolution)
			{
				windowWidth  = context->glfw.dims[0];
				windowHeight = context->glfw.dims[1];
				tex.recreateImage({ windowWidth, windowHeight });
				tex.bindToUnit(0);
				tex.bindToImage(0, TEX_IMAGE_WRITE_ONLY_ACCESS);


				camViewportWidth = context->persp.__.aspectRatio * sceneDescription->transform.viewport.y;
				ssbo.bind();
				ssbo.update(offsetof(CameraTransform, viewport.x), { 
					&camViewportWidth,
					4,
					{}
				});
				ssbo.unbind();

				invocDims = recomputeDispatchSize({ windowWidth, windowHeight });
			}
		}


        running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
        focused = !context->glfw.minimized();
        paused  = paused ^ isKeyPressed(KeyCode::P);
		refresh[0] = isKeyPressed(KeyCode::NUM8);
		refresh[1] = isKeyPressed(KeyCode::NUM9);
        refresh[2] = isKeyPressed(KeyCode::NUM0);
		changedResolution = context->glfw.windowSizeChanged();


        context->glfw.procOngoingEvents();
		++context->frameIndex;
	}
	context->glfw.close();


	compute.destroy();
	shader.destroy();

	ssbo.destroy();
	tex.destroy();
	vbo.destroy();
	ibo.destroy();
	context->glfw.destroy();
	free(fullShaderPaths[0].first);
	free(sceneDescription);
	return 0;
}


/*


Image isn't rendered to screen, 
both in basicpp.cpp and raytrace.cpp, 
Problem is with (probably) VertexFragment Shader, 
as reloading shows the texture again (but black). 
Further Testing Needed.

*/