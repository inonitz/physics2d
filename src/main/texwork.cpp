#include "texwork.hpp"
#include "gl/shader2.hpp"
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"




int make_texture_resize_work()
{
    auto* ctx = getGlobalContext();
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};
	std::vector<f32> vboData =
	{
		-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
		-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
		1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
	};

	std::vector<u32> iboData =
	{
		0, 2, 1,
		0, 3, 2
	};
	u64 temporary;
	u32 w = 1280, h = 720;
	u8  sphereCount = 13;
	f32 viewportSize = 2.0f;
	i32 samplesPerPixel       = 40;
	i32 diffuseRecursionDepth = 8;
	f32 randomNorm = randnorm32f();
	std::array<const char*, 3> shaderFiles = {
		"C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/shader.vert",
		"C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/shader.frag",
		"C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/shader_diffuse.comp"
	};
	std::vector<Material> 				  materials;
	std::vector<ObjectMaterialDescriptor> objToMaterialMap;
    Program basic_shader, basic_compute;
	ShaderStorageBuffer sceneData, materialBuffer, objectMaterialMeta;
    VertexArray vao;
    Buffer      vbo, ibo;
    TextureBuffer     texbuf;
	SceneData*        sceneDescription = nullptr;
    ComputeGroupSizes invocDims;
	boolStates 		  programStates;
	memset(&programStates, 0x00, sizeof(programStates.s));


	temporary = sizeof(SceneData) + (sphereCount - 1) * sizeof(Sphere);
	sceneDescription = __scast(SceneData*, malloc(temporary));
	sceneDescription->transform = {
		math::vec3f{ 0.0f },
		math::vec2f{ 0.0f },
		1.0f, 
		0
	};
	sceneDescription->curr_size = 4;
	sceneDescription->max_size  = 13;
    sceneDescription->objects[0] = Sphere{ { 0.0f, -100.5f, -1.0f, 100.0f} };
    sceneDescription->objects[1] = Sphere{ { 0.0f,    0.0f, -1.0f,   0.5f} };
    sceneDescription->objects[2] = Sphere{ {-1.0f,    0.0f, -1.0f,   0.5f} };
    sceneDescription->objects[3] = Sphere{ { 1.0f,    0.0f, -1.0f,   0.5f} };
	materials = {
		{0.8f, 0.8f, 0.0f, 0   },
		{0.7f, 0.3f, 0.3f, 0   },
		{0.8f, 0.8f, 0.8f, 0.0f},
		{0.8f, 0.6f, 0.2f, 0.0f},
	};
	objToMaterialMap = {
		ObjectMaterialDescriptor{ MATERIAL_LAMBERTIAN, 0x00, 0},
		ObjectMaterialDescriptor{ MATERIAL_LAMBERTIAN, 0x00, 1},
		ObjectMaterialDescriptor{ MATERIAL_LAMBERTIAN,      0x00, 2},
		ObjectMaterialDescriptor{ MATERIAL_LAMBERTIAN,      0x00, 3}
	};



    ctx->glfw.create(w, h, eventFuncs);
	debug( /* Enable Advanced OpenGL Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
    glClearColor(0.45f, 1.05f, 0.60f, 1.00f);


	basic_shader.createFrom({ { shaderFiles[0], GL_VERTEX_SHADER   }, { shaderFiles[1], GL_FRAGMENT_SHADER } });
    basic_compute.createFrom({{ shaderFiles[2], GL_COMPUTE_SHADER  }});
    

    vbo.create(BufferDescriptor{ vboData.data(), 4u, 
			{{
				{ GL_FLOAT, 3 },
				{ GL_FLOAT, 2 }
			}}
		},
		GL_STREAM_DRAW
	);
	ibo.create({ iboData.data(), __scast(u32, iboData.size()), 
			{{
				{ GL_UNSIGNED_INT, 1 },
			}}
		},
		GL_STREAM_DRAW
	);
    vao.create(vbo, ibo);


    texbuf.create({
		{ w, h },
		nullptr,
		{4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{
			{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  },
		}
	});


	sceneDescription->transform.viewport = {
		viewportSize * ctx->glfw.aspectRatio<f32>(),
		viewportSize
	};
	sceneData.create(
		{ 
			sceneDescription, 
			__scast(u32, temporary), 
			VertexDescriptor::defaultVertex()
		},  
		GL_DYNAMIC_DRAW
	);
	materialBuffer.create(
		{
			materials.data(),
			__scast(u32, materials.size()),
			VertexDescriptor::defaultVertex()
		},
		GL_DYNAMIC_DRAW
	);
	objectMaterialMeta.create(
		{
			objToMaterialMap.data(),
			__scast(u32, objToMaterialMap.size()),
			VertexDescriptor::defaultVertex()
		},
		GL_DYNAMIC_DRAW
	);
	sceneData.setBindingIndex(1);
	materialBuffer.setBindingIndex(2);
	objectMaterialMeta.setBindingIndex(3);


    invocDims = computeDispatchSize({ w, h });
    basic_compute.resizeLocalWorkGroup(0, invocDims.localGroup);
    ifcrashdo(basic_shader.compile()  == GL_FALSE, debug_message("Problem Compiling Vertex-Fragment Shader\n"));
	ifcrashdo(basic_compute.compile() == GL_FALSE, debug_message("Problem Compiling Compute Shader\n")		  );
	programStates.running = true;
	programStates.focused = true;
	// texbuf.bindToUnit(0);
    while(programStates.running) 
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ctx->glfw.procUpcomingEvents();
        renderUI(
			diffuseRecursionDepth,
			samplesPerPixel,
			materials,
			programStates.bytes[5],
			randomNorm,
			invocDims.dispatchGroup
		);


		if(!programStates.paused) 
		{
			/* Compute Shader Pass */
			basic_compute.bind();
			sceneData.bind();
			materialBuffer.bind();
			objectMaterialMeta.bind();
			basic_compute.uniform1f("u_dt", ctx->glfw.time_dt());
			basic_compute.uniform1f("u_rand", randomNorm);
			basic_compute.uniform1i("u_samplesPpx", samplesPerPixel);
			basic_compute.uniform1i("u_recurseDepth", diffuseRecursionDepth);
			texbuf.bindToImage(1, TEX_IMAGE_WRITE_ONLY_ACCESS);
			glDispatchCompute(
				invocDims.dispatchGroup.x, 
				invocDims.dispatchGroup.y,
				invocDims.dispatchGroup.z
			);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);


			/* Vertex Shader Pass */
			basic_shader.bind();
			vao.bind();
			texbuf.bindToUnit(0);
			basic_shader.uniform1i("tex", 0);
			glDrawElements(GL_TRIANGLES, iboData.size(), GL_UNSIGNED_INT, 0);


			/* Check for specific state changes */
			if(programStates.windowSizeChange)
			{
				w = ctx->glfw.dims[0];
				h = ctx->glfw.dims[1];
				texbuf.recreateImage({ w, h });

				invocDims = computeDispatchSize({ w, h });
				sceneDescription->transform.viewport.x = ctx->glfw.aspectRatio<f32>() * viewportSize;
				sceneData.update(offsetof(SceneData, transform.viewport.x), { &sceneDescription->transform.viewport.x, 4, {} });
				
				basic_compute.resizeLocalWorkGroup(0, invocDims.localGroup);
				programStates.refreshCompute = basic_compute.compile();
			}

			if(programStates.refreshSSBOs)
			{
				mark(); materialBuffer.bind();
				mark(); materialBuffer.update(sizeof(Material) * programStates.bytes[5], 
				{ 
						&materials[programStates.bytes[5]], 
						__scast(u32, sizeof(Material)), 
						{} 
				});
				mark();
			}


			if(programStates.refreshCompute) {
				basic_compute.refreshShaderSource(0);
				basic_compute.resizeLocalWorkGroup(0, invocDims.localGroup);
				programStates.refreshCompute = basic_compute.compile();
			}

			if(programStates.refreshShader) {
				basic_shader.refreshShaderSource(0);
				basic_shader.refreshShaderSource( 1);
				programStates.refreshShader = basic_shader.compile();
			
			}
		}


		programStates.s[0]  = !ctx->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
		programStates.s[1] ^= isKeyPressed(KeyCode::P);
		programStates.s[2]  = !ctx->glfw.minimized();
		programStates.s[3]  = isKeyPressed(KeyCode::NUM5);
		programStates.s[4]  = isKeyPressed(KeyCode::NUM6);
        programStates.s[6]  = ctx->glfw.windowSizeChanged();
        ctx->glfw.procOngoingEvents();
    }
    ctx->glfw.close();
    texbuf.destroy();
    vao.destroy();
    vbo.destroy();
    ibo.destroy();
    basic_compute.destroy();
    basic_shader.destroy();
    ctx->glfw.destroy();
    

    return 0;
}


/* 
	Merged FixBlackScreen & Diffusion Branches.
	Next:
		* Materials
		* reflectivity
		* dynamic objects
		* Polygons?
		* dynamic camera?
*/