#pragma once
#include <glad/glad.h>
#include <ImGui1896/imgui.h>
#include "context.hpp"
#include "util/random.hpp"




struct ComputeGroupSizes
{
	math::vec3u localGroup;
	math::vec3u dispatchGroup;
};


union boolStates {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic warning "-Wgnu-anonymous-struct"
	struct pack { /* don't care this is a gnu extension, go to hell (most compilers support this) */
		bool running;
		bool paused;
		bool focused;
		bool refreshShader;
		bool refreshCompute;
		bool refreshSSBOs;
		bool windowSizeChange;
	};
	#pragma GCC diagnostic pop
	bool s[7];
	u8   bytes[7];
};


void setupShaderPaths(std::array<char*, 3>& fullShaderNames);
ComputeGroupSizes computeDispatchSize(math::vec2u const& dims);
void renderUI(
	i32& 			       diffuseRecursionDepth,
	i32& 			       samplesPerPixel,
	std::vector<Material>& objectMaterials,
	u8&                    refreshMaterials,
	PVMTransform&          sceneCamera,
	f32&       	  		   randomFloat,
	math::vec3u const& 	   computeDispatchSize
);
int make_texture_resize_work();




inline void setupShaderPaths(std::array<char*, 3>& fullShaderNames)
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


inline ComputeGroupSizes computeDispatchSize(math::vec2u const& dims)
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