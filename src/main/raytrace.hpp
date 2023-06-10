#pragma once
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include "util/random.hpp"
#include "../context.hpp"




static inline std::vector<f32> squareVertices =
{
	-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
	-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
};

static inline std::vector<u32> squareIndices =
{
	0, 2, 1,
	0, 3, 2
};


struct ComputeGroupSizes
{
	math::vec3u localGroup;
	math::vec3u dispatchGroup;
};


inline ComputeGroupSizes recomputeDispatchSize(math::vec2u const& dims);
inline void renderImGui(
	SceneData*   scene,
	f32&         out_deltaTime,
	i32& 		 out_samples, 
	i32& 		 recursionDepth,
	math::vec4f& sample_randf, 
	math::vec3u const& computeDispatchSize
);

int raytracer();







inline ComputeGroupSizes recomputeDispatchSize(math::vec2u const& dims)
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
	
	// debug_messagefmt("recomputeDispatchSize:\n    Local workgroup Size of { %u %u %u }\n    Workgroup Dispatch Size { %u %u %u }\n", 
	// 	localWorkgroupSize.x, localWorkgroupSize.y, 1,
	// 	(u32)tmp_cvt.x, 	  (u32)tmp_cvt.y, 1
	// );
	
	
	return ComputeGroupSizes{
		localWorkgroupSize,
		{ __scast(u32, tmp_cvt.x), __scast(u32, tmp_cvt.y), 1u }	
	};
}




void renderImGui(
	SceneData*   scene,
	f32&         out_deltaTime,
	i32& 		 out_samples, 
	i32& 		 recursionDepth,
	math::vec4f& sample_randf, 
	math::vec3u const& computeDispatchSize
) {
	auto* ctx = getGlobalContext();
	std::array<i32, 2> windowDims = ctx->glfw.dims;
	static i32 work_grp_inf[7];
	f32 dt = ctx->glfw.time_dt();
	

	__once(
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0,  &work_grp_inf[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1,  &work_grp_inf[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2,  &work_grp_inf[2]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0,   &work_grp_inf[3]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1,   &work_grp_inf[4]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2,   &work_grp_inf[5]);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inf[6]);
	);
	out_deltaTime = dt;
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
	ImGui::Text("Random Float  For Pixel Samples =   %.05f                ", sample_randf.x);

	if(ImGui::Button("Refresh =>"))
		sample_randf = { randnorm32f(), randnorm32f(), randnorm32f(), randnorm32f() };
	ImGui::SameLine();
	ImGui::Text("Random Vec4: { %.05f, %.05f, %.05f, %.05f }", sample_randf.x, sample_randf.y, sample_randf.z, sample_randf.w);


	ImGui::SliderInt("Random Samples Per Pixel ", &out_samples   , 0, 128);
	ImGui::SliderInt("Diffusion-Recursion Depth", &recursionDepth, 0, 64 );
	ImGui::SliderFloat("Camera Viewport Length", &scene->transform.viewport.y, 0.0f, 10.0f);
	ImGui::EndGroup();
	return;
}