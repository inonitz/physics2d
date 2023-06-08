#include "texwork.hpp"
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include <string.h>
#include "context.hpp"
#include "gl/shader2.hpp"
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"




void setupShaderPaths(std::array<char*, 3>& fullShaderNames)
{
	const char* shaderPath[2] = { "C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/", "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/" };
	const char* shaderName[3] = { "shader.vert", "shader.frag", "shader.comp" };
	size_t 		stringLengths[5] = { 
		strlen(shaderPath[0]),
		strlen(shaderPath[1]),
		strlen(shaderName[0]),
		strlen(shaderName[1]),
		strlen(shaderName[2])
	};
	char* stringArray = (char*)malloc(stringLengths[0] + stringLengths[1] + stringLengths[2] + stringLengths[3] + stringLengths[4]);
	char* ptrOffset   = stringArray;


	u8 selectShaderPath = 1;
	for(size_t i = 0; i < 3; ++i) {
		fullShaderNames[i] = ptrOffset;
		memcpy(ptrOffset, shaderPath[selectShaderPath], stringLengths[selectShaderPath]);
		ptrOffset += stringLengths[selectShaderPath];
		memcpy(ptrOffset, shaderName[i], stringLengths[2 + i] + 1);
		ptrOffset += stringLengths[2 + i] + 1; /* +1 because I want to include null-terminator when moving to new shaderName. */
	}
	debug_messagefmt("Shader paths are: \n%s\n%s\n%s\n", 
		fullShaderNames[0], 
		fullShaderNames[1], 
		fullShaderNames[2]
	);


	return; /* use fullShaderNames[0] to de-alloc the block-of-memory */
}




struct ComputeGroupSizes
{
	math::vec3u localGroup;
	math::vec3u dispatchGroup;
};


ComputeGroupSizes computeDispatchSize(math::vec2u const& dims)
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
	
	
	return ComputeGroupSizes{
		localWorkgroupSize,
		{ __scast(u32, tmp_cvt.x), __scast(u32, tmp_cvt.y), 1u }
	};
}


void renderUI(math::vec3u const& computeDispatchSize)
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
	ImGui::EndGroup();
	return;
}




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




int make_texture_resize_work()
{
    auto* ctx = getGlobalContext();
	std::array<char*, 3> shaderFiles;
	u32 w = 1280, h = 720;
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};
    Program basic_shader, basic_compute;
    VertexArray vao;
    Buffer      vbo, ibo;
    TextureBuffer     texbuf;
    ComputeGroupSizes invocDims;
    bool programStates[5] = {false, false, false, false, true };



	setupShaderPaths(shaderFiles);
	mark();
    ctx->glfw.create(w, h, eventFuncs);
	mark();
    glClearColor(0.45f, 1.05f, 0.60f, 1.00f);

    basic_shader.createFrom({
        { shaderFiles[0], GL_VERTEX_SHADER   },
        { shaderFiles[1], GL_FRAGMENT_SHADER }
    });
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


    invocDims = computeDispatchSize({ w, h });
    basic_compute.resizeLocalWorkGroup(0, invocDims.localGroup);

    
    ifcrashdo(basic_shader.compile()  == GL_FALSE, debug_message("Problem Compiling Vertex-Fragment Shader\n"));
	ifcrashdo(basic_compute.compile() == GL_FALSE, debug_message("Problem Compiling Compute Shader\n")		  );
    texbuf.bindToUnit(0);
    while(!ctx->glfw.shouldClose() && programStates[4]) 
    {
        ctx->glfw.procUpcomingEvents();
        renderUI(invocDims.dispatchGroup);


        basic_compute.bind();
        texbuf.bindToImage(1, TEX_IMAGE_WRITE_ONLY_ACCESS);
        glDispatchCompute(
            invocDims.dispatchGroup.x, 
            invocDims.dispatchGroup.y,
            invocDims.dispatchGroup.z
        );
        glMemoryBarrier(GL_ALL_BARRIER_BITS);


        basic_shader.bind();
        vao.bind();
        texbuf.bindToUnit(0);
        basic_shader.uniform1i("tex", 0);
        glDrawElements(GL_TRIANGLES, iboData.size(), GL_UNSIGNED_INT, 0);


		if(programStates[2]) {
			basic_shader.refreshShaderSource(0);
			basic_shader.refreshShaderSource(1);
			programStates[4] = basic_shader.compile();
		}


        if(programStates[3])
        {
            w = ctx->glfw.dims[0];
            h = ctx->glfw.dims[1];
            texbuf.recreateImage({ w, h });


            invocDims = computeDispatchSize({ w, h });
            basic_compute.resizeLocalWorkGroup(0, invocDims.localGroup);
            programStates[4] = basic_compute.compile();
        }


		programStates[2] = isKeyPressed(KeyCode::NUM5);
        programStates[3] = ctx->glfw.windowSizeChanged();
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
    free(shaderFiles[0]);
    return 0;
}