#include "texwork.hpp"
#include "gl/shader2.hpp"
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"
#include "camera.hpp"




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
	__unused f32 viewportSize = 2.0f;
	i32 samplesPerPixel       = 40;
	i32 diffuseRecursionDepth = 8;
	f32 randomNorm = randnorm32f();
	// std::array<const char*, 3> shaderFiles = {
	// 	"C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/shader.vert",
	// 	"C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/shader.frag",
	// 	"C:/CTools/Projects/mglw-strip/assets/shaders/raytrace/shader_diffuse.comp"
	// };
	std::array<const char*, 3> shaderFiles = {
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.vert",
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.frag",
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader_diffuse.comp"
	};
	std::vector<Material> 				  materials;
	std::vector<ObjectMaterialDescriptor> objToMaterialMap;
    Program vertex_fragment, compute;
	ShaderStorageBuffer sceneData, materialBuffer, objectMaterialMeta;
    VertexArray screenTexVAO;
    Buffer      screenTexVBO, screenTexIBO;
    TextureBuffer     screenTexBuffer;
	SceneDataV2*      sceneDescription = nullptr;
	PVMTransform      sceneTransform;
    ComputeGroupSizes invocDims;
	boolStates 		  programStates;
	memset(&programStates, 0x00, sizeof(programStates.s));


	temporary = sizeof(SceneDataV2) + (sphereCount - 1) * sizeof(Sphere);
	sceneDescription = __scast(SceneDataV2*, malloc(temporary));
	sceneDescription->curr_size = 4;
	sceneDescription->max_size  = 13;
    sceneDescription->objects[0] = Sphere{ { 0.0f, -100.5f, -1.0f, 100.0f} };
    sceneDescription->objects[1] = Sphere{ { 0.0f,    0.0f, -1.0f,   0.5f} };
    sceneDescription->objects[2] = Sphere{ {-1.0f,    0.0f, -1.0f,   0.5f} };
    sceneDescription->objects[3] = Sphere{ { 1.0f,    0.0f, -1.0f,   0.5f} };
	materials = {
		{0.8f, 0.8f, 0.0f, 0   },
		{0.1f, 0.2f, 0.5f, 0   },
		{0.0f, 0.0f, 0.0f, +1.5f},
		{0.8f, 0.6f, 0.2f, +0.0f},
	};
	objToMaterialMap = {
		ObjectMaterialDescriptor{ MATERIAL_LAMBERTIAN, 0x00, 0},
		ObjectMaterialDescriptor{ MATERIAL_LAMBERTIAN, 0x00, 1},
		ObjectMaterialDescriptor{ MATERIAL_DIELECTRIC,      0x00, 2},
		ObjectMaterialDescriptor{ MATERIAL_METAL,      0x00, 3}
	};




    ctx->glfw.create(w, h, eventFuncs);
	debug( /* Enable Advanced OpenGL Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
    glClearColor(0.45f, 1.05f, 0.60f, 1.00f);


	vertex_fragment.createFrom({ { shaderFiles[0], GL_VERTEX_SHADER   }, { shaderFiles[1], GL_FRAGMENT_SHADER } });
    compute.createFrom({{ shaderFiles[2], GL_COMPUTE_SHADER  }});
    

    screenTexVBO.create(BufferDescriptor{ vboData.data(), 4u, 
			{{
				{ GL_FLOAT, 3 },
				{ GL_FLOAT, 2 }
			}}
		},
		GL_STREAM_DRAW
	);
	screenTexIBO.create({ iboData.data(), __scast(u32, iboData.size()), 
			{{
				{ GL_UNSIGNED_INT, 1 },
			}}
		},
		GL_STREAM_DRAW
	);
    screenTexVAO.create(screenTexVBO, screenTexIBO);


    screenTexBuffer.create({
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



	sceneTransform.create(90.0f, 0.1f, 100.0f);
	sceneTransform.recalculateProjection();
	sceneTransform.writeFinalData(sceneDescription->transform);


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
			__scast(u32, materials.size() * sizeof(Material)),
			VertexDescriptor::defaultVertex()
		},
		GL_DYNAMIC_DRAW
	);
	objectMaterialMeta.create(
		{
			objToMaterialMap.data(),
			__scast(u32, objToMaterialMap.size() * sizeof(ObjectMaterialDescriptor)),
			VertexDescriptor::defaultVertex()
		},
		GL_DYNAMIC_DRAW
	);
	sceneData.setBindingIndex(1);
	materialBuffer.setBindingIndex(2);
	objectMaterialMeta.setBindingIndex(3);


    invocDims = computeDispatchSize({ w, h });
    compute.resizeLocalWorkGroup(0, invocDims.localGroup);
    ifcrashdo(vertex_fragment.compile()  == GL_FALSE, debug_message("Problem Compiling Vertex-Fragment Shader\n"));
	ifcrashdo(compute.compile() == GL_FALSE, debug_message("Problem Compiling Compute Shader\n")		  );
	programStates.running = true;
	programStates.focused = true;
    while(programStates.running) 
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ctx->glfw.procUpcomingEvents();
        renderUI(
			diffuseRecursionDepth,
			samplesPerPixel,
			materials,
			programStates.bytes[5],
			sceneTransform,
			randomNorm,
			invocDims.dispatchGroup
		);


		if(!programStates.paused) 
		{
			if(programStates.windowSizeChange)
			{
				w = ctx->glfw.dims[0];
				h = ctx->glfw.dims[1];
				screenTexBuffer.recreateImage({ w, h });

				invocDims = computeDispatchSize({ w, h });
				compute.resizeLocalWorkGroup(0, invocDims.localGroup);
				programStates.refreshCompute = compute.compile();
			}

			if(programStates.windowSizeChange || programStates.bytes[5] & 0xe0) {
				sceneTransform.m_persp.__.aspectRatio = ctx->glfw.aspectRatio<f32>();
				sceneTransform.recalculateProjection();
			}
			sceneTransform.onUpdate(ctx->glfw.time_dt());
			sceneTransform.writeFinalData(sceneTransform.m_data);


			/* Check for changes in SSBO's */
			if(programStates.bytes[5] > 0 && programStates.bytes[5] < 0x20)
			{
				materialBuffer.bind();
				materialBuffer.update(
					__scast(u32, sizeof(Material) * (programStates.bytes[5]-1) ),
					{
						&materials[programStates.bytes[5] - 1],
						__scast(u32, sizeof(Material)),
						{}
				});
			}
			sceneData.bind();
			sceneData.update(
				offsetof(SceneDataV2, transform), 
				{ 
					&sceneTransform.m_data,
					sizeof(CameraTransformV2),
					{}
			});


			if(programStates.refreshCompute) {
				compute.refreshShaderSource(0);
				compute.resizeLocalWorkGroup(0, invocDims.localGroup);
				programStates.refreshCompute = compute.compile();
			}

			if(programStates.refreshShader) {
				vertex_fragment.refreshShaderSource(0);
				vertex_fragment.refreshShaderSource( 1);
				programStates.refreshShader = vertex_fragment.compile();
			}


			/* Compute Shader Pass */
			compute.bind();
			sceneData.bind();
			materialBuffer.bind();
			objectMaterialMeta.bind();
			compute.uniform1f("u_dt", ctx->glfw.time_dt());
			compute.uniform1f("u_rand", randomNorm);
			compute.uniform1i("u_samplesPpx", samplesPerPixel);
			compute.uniform1i("u_recurseDepth", diffuseRecursionDepth);
			screenTexBuffer.bindToImage(1, TEX_IMAGE_WRITE_ONLY_ACCESS);
			glDispatchCompute(
				invocDims.dispatchGroup.x, 
				invocDims.dispatchGroup.y,
				invocDims.dispatchGroup.z
			);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);


			/* Vertex Shader Pass */
			vertex_fragment.bind();
			screenTexVAO.bind();
			screenTexBuffer.bindToUnit(0);
			vertex_fragment.uniform1i("tex", 0);
			glDrawElements(GL_TRIANGLES, iboData.size(), GL_UNSIGNED_INT, 0);
		}


		programStates.s[0]  = !ctx->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
		programStates.s[1] ^= isKeyPressed(KeyCode::P);
		programStates.s[2]  = !ctx->glfw.minimized();
		programStates.s[3]  = isKeyPressed(KeyCode::NUM5);
		programStates.s[4]  = isKeyPressed(KeyCode::NUM6);
		programStates.s[5]  = false;
        programStates.s[6]  = ctx->glfw.windowSizeChanged();
        ctx->glfw.procOngoingEvents();
    }
    ctx->glfw.close();
    screenTexBuffer.destroy();
    screenTexVAO.destroy();
    screenTexVBO.destroy();
    screenTexIBO.destroy();
    compute.destroy();
    vertex_fragment.destroy();
    ctx->glfw.destroy();
    

    return 0;
}




void renderUI(
	i32& 			   	   diffuseRecursionDepth,
	i32& 			   	   samplesPerPixel,
	std::vector<Material>& objectMaterials,
	u8&                    refreshMaterials,
	PVMTransform&          sceneTransformMatrices,
	f32&       	   		   randomFloat,
	math::vec3u const& 	   computeDispatchSize
) {
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


	if(ImGui::Button("Refresh =>")) randomFloat = randnorm32f();
	ImGui::SameLine();
	ImGui::Text("Generated Float: %.05f", randomFloat);
	ImGui::SliderInt("Samples Per Pixel ", &samplesPerPixel   , 0, 128);
	ImGui::SliderInt("Ray Trace Depth   ", &diffuseRecursionDepth, 0, 64 );
	refreshMaterials  = 0x80 * ImGui::SliderFloat("Field Of View   		  ", &sceneTransformMatrices.m_persp.__.fieldOfView, 10.0f,  150.0f);
	refreshMaterials += 0x40 * ImGui::SliderFloat("Near Clipping Plane     ", &sceneTransformMatrices.m_persp.__.nearClip, 0.001f,  5.0f);
	refreshMaterials += 0x20 * ImGui::SliderFloat("Far  Clipping Plane     ", &sceneTransformMatrices.m_persp.__.farClip, 10.0f,  100.0f);
	ImGui::EndGroup();


	struct CharArray { char __[20]; };
	static std::vector<CharArray> MaterialNames{objectMaterials.size()};
	__once(for(size_t i = 0; i < objectMaterials.size(); ++i) { /* Currently the amount of materials is static so this is fine. */
			sprintf(MaterialNames[i].__, "Material %2d", __scast(u32, i));
	});
	ImGui::BeginChild("Object Materials");
	for(size_t i = 0; i < objectMaterials.size(); ++i) {
		if(ImGui::ColorEdit4(MaterialNames[i].__, &objectMaterials[i].x))
			refreshMaterials = __scast(u8, i+1);
	}
	ImGui::EndChild();


	return;
}


/* 
	Merged FixBlackScreen & Diffusion Branches.
	Next:
		* dynamic objects
		* Polygons?
*/



/*
	Pseudocode for different material types:


	if world.hit(ray, possibleHit):
		if scatter(rayAtUV, possibleHit, attenuation, scatteredRay) 
			return attenuation * getRayColor(scatteredRay, --depth);
		return 0


	* Lambertian {
		worldRayIntersection(cameraRayAtUV, possibleHit);
		cont = !isinf(possibleHit.t);
		finalColor = vec3(1.0f);

		if(cont) {
			scatterDir = possibleHit.normal + simpleRandUnitSphere(texelCoord, i);
			if(all(near_zero(scatter_dir)))
				scatterDir = possibleHit.normal;

			cameraRayAtUV.origin = rayAt(cameraRayAtUV, possibleHit.t);
			cameraRayAtUV.dir    = scatterDir;
			attenuatedColor = materials[  objectToMaterial[possibleHit.objectIndex]  ].xyz;
			finalColor *= attenuatedColor;
		}
	}


	* Metal {
		worldRayIntersection(cameraRayAtUV, possibleHit);
		cont = !isinf(possibleHit.t);
		finalColor = vec3(1.0f);

		if(cont) {
			reflectDir = reflectRay(normalize(cameraRayAtUV.dir)), possibleHit.normal);
			
			cameraRayAtUV.origin = rayAt(cameraRayAtUV, possibleHit.t);
			cameraRayAtUV.dir    = reflectDir;
			attenuatedColor = materials[  objectToMaterial[possibleHit.objectIndex]  ].xyz;
			
			// stop the loop if scatter wasn't valid
			cont = dot(reflectDir, possibleHit.normal) > 0);
			finalColor *= mix(vec3(0.0f), attenuatedColor, bvec3(cont));
		}
	}


	* Fuzzy Metal {
		worldRayIntersection(cameraRayAtUV, possibleHit);
		cont = !isinf(possibleHit.t);
		finalColor = vec3(1.0f);
		materialVec = vec4(0.0f);

		if(cont) {
			materialVec = materials[  objectToMaterial[possibleHit.objectIndex]  ];


			reflectDir = reflectRay(normalize(cameraRayAtUV.dir)), possibleHit.normal);
			cameraRayAtUV.origin = rayAt(cameraRayAtUV, possibleHit.t);
			cameraRayAtUV.dir    = reflectDir + materialVec.w * simpleRandUnitSphere(texelCoord, i);
			attenuatedColor = materialVec.xyz;

			// stop the loop if scatter wasn't valid
			cont = dot(reflectDir, possibleHit.normal) > 0;
			finalColor *= mix(vec3(0.0f), attenuatedColor, bvec3(cont));
		}
	}


	* dielectric {
		worldRayIntersection(cameraRayAtUV, possibleHit);
		cont = !isinf(possibleHit.t);
		finalColor = vec3(1.0f);

		if(cont) {
			front_face = dot(cameraRayAtUV.dir, possibleHit.normal) > 0);
			refract_ratio = materials[  objectToMaterial[possibleHit.objectIndex]  ].w;
			refract_ratio = mix(refract_ratio, 1.0f/refract_ratio, front_face);

			unitRay = normalize(cameraRayAtUV.dir);
			cos = dot(unitRay, possibleHit.normal);
			sin = sqrt(1 - cos * cos);

			bool shouldReflect = (refract_ratio * sin > 1.0f) || (reflectance(cos, refract_ratio) > random(texelCoord * u_rand));
			cameraRayAtUV.origin = rayAt(cameraRayAtUV, possibleHit.t);
			cameraRayAtUV.dir = mix(
				refractRay(unitRay, possibleHit.normal, refract_ratio),
				reflectRay(unitRay, possibleHit.normal),
				bvec3(shouldReflect) // if refraction not possible
			);
		}
	}


*/


	// ImGui::BeginGroup();
	// ImGui::BulletText("Inverse Projection: {\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n}\n",
	// 	sceneTransformMatrices.m_persp.constref().mem[0],
	// 	sceneTransformMatrices.m_persp.constref().mem[1],
	// 	sceneTransformMatrices.m_persp.constref().mem[2],
	// 	sceneTransformMatrices.m_persp.constref().mem[3],
	// 	sceneTransformMatrices.m_persp.constref().mem[4],
	// 	sceneTransformMatrices.m_persp.constref().mem[5],
	// 	sceneTransformMatrices.m_persp.constref().mem[6],
	// 	sceneTransformMatrices.m_persp.constref().mem[7],
	// 	sceneTransformMatrices.m_persp.constref().mem[8],
	// 	sceneTransformMatrices.m_persp.constref().mem[9],
	// 	sceneTransformMatrices.m_persp.constref().mem[10],
	// 	sceneTransformMatrices.m_persp.constref().mem[11],
	// 	sceneTransformMatrices.m_persp.constref().mem[12],
	// 	sceneTransformMatrices.m_persp.constref().mem[13],
	// 	sceneTransformMatrices.m_persp.constref().mem[14],
	// 	sceneTransformMatrices.m_persp.constref().mem[15]
	// );
	// ImGui::BulletText("View Matrix: {\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n    \
	// 	{ %03.5f, %03.5f, %03.5f, %03.5f },\n}\n",
	// 	sceneTransformMatrices.m_view.constref().mem[0],
	// 	sceneTransformMatrices.m_view.constref().mem[1],
	// 	sceneTransformMatrices.m_view.constref().mem[2],
	// 	sceneTransformMatrices.m_view.constref().mem[3],
	// 	sceneTransformMatrices.m_view.constref().mem[4],
	// 	sceneTransformMatrices.m_view.constref().mem[5],
	// 	sceneTransformMatrices.m_view.constref().mem[6],
	// 	sceneTransformMatrices.m_view.constref().mem[7],
	// 	sceneTransformMatrices.m_view.constref().mem[8],
	// 	sceneTransformMatrices.m_view.constref().mem[9],
	// 	sceneTransformMatrices.m_view.constref().mem[10],
	// 	sceneTransformMatrices.m_view.constref().mem[11],
	// 	sceneTransformMatrices.m_view.constref().mem[12],
	// 	sceneTransformMatrices.m_view.constref().mem[13],
	// 	sceneTransformMatrices.m_view.constref().mem[14],
	// 	sceneTransformMatrices.m_view.constref().mem[15]
	// );
	// ImGui::BulletText("Camera Position: { %03.5f, %03.5f, %03.5f, %03.5f }\n",
	// 	sceneTransformMatrices.m_view.position().x,
	// 	sceneTransformMatrices.m_view.position().y,
	// 	sceneTransformMatrices.m_view.position().z,
	// 	sceneTransformMatrices.m_view.position().w
	// );
	// math::vec3f tmp = sceneTransformMatrices.m_view.lookingAt();
	// ImGui::BulletText("Camera LookAt: { %03.5f, %03.5f, %03.5f, %03.5f }\n",
	// 	tmp.x, tmp.y, tmp.z, tmp.w
	// );
	// ImGui::EndGroup();
