#pragma once
#include <string_view>
#include <vector>
#include "vec.hpp"




struct ShaderData {
	const char* filepath;
	u32 	    type;
	u32 		id;

	ShaderData(const char* fp, u32 glTypeEnum) : filepath{fp}, type{glTypeEnum}, id{DEFAULT32} {}
};


struct BufferData {
	char*  data;
	size_t size;
};


constexpr const char* shaderTypeToString(u32 type);




typedef struct __ShaderProgramV1 
{
public:
	u32 m_id = DEFAULT32;

private:
	using shaderContents = std::vector<char>;
	using shaderType 	 = u32;
	using loadedShader   = std::pair<BufferData, shaderType>;

    /* for CREATE_UNIFORM_FUNCTION_DEFINITON macro */
    using array2f = std::array<f32, 2>;
    using array3f = std::array<f32, 3>;
    using array4f = std::array<f32, 4>;
    using array2i = std::array<i32, 2>;
    using array3i = std::array<i32, 3>;
    using array4i = std::array<i32, 4>;
    using array2u = std::array<u32, 2>;
    using array3u = std::array<u32, 3>;
    using array4u = std::array<u32, 4>;


	static inline std::array<char, 1024> genericErrorLog;
	std::vector<ShaderData> 	  shaders;
	std::vector<shaderContents>   sources;
	/* 
		Honestly this (^^^) 'sources' variable makes more sense if we listened to file changes 
		OR if we got an arg that specified which shader changed.
		That being said, it's still nice seeing shader contents when debugging, so i'll keep this for now,
		even though this is not so useful.
	*/


	bool loadShader(ShaderData& init, BufferData& loadedShader);
	void createProgram();

public:
	void fromFiles(std::vector<ShaderData> const& shaderInfo);
	void fromBuffers(std::vector<loadedShader> const& buffers);


    void bind()   const;
    void unbind() const;
    void destroy();
	__force_inline bool success() const { return m_id != DEFAULT32; } 



	void reload(std::vector<loadedShader> const& buffers = {})
	{
		if(m_id != DEFAULT32) destroy();
		/* 
			I could probably pick the common code-path from both of these functions
			and get rid of the if statement, but this is fine for now. 
		*/
		if(buffers.size() != 0)
		{
			fromBuffers(buffers);
		} else {
			fromFiles(shaders);
		}
		return;
	}

	void fromFilesCompute(
		std::vector<ShaderData> const& shaderInfo, 
		math::vec3u 			const& localWorkgroupSize = { 32, 48, 1 }
	);
	void reloadCompute(math::vec3u const& localWorkgroupSize = { 32, 48, 1 });


#define CREATE_UNIFORM_FUNCTION_DEFINITON(TypeSpecifier, arg0) [[maybe_unused]] void uniform##TypeSpecifier(std::string_view const& name, arg0);
	CREATE_UNIFORM_FUNCTION_DEFINITON(1f,  f32 v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(1i,  i32 v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(1ui, u32 v)

	CREATE_UNIFORM_FUNCTION_DEFINITON(2f,  array2f const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2i,  array2i const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2ui, array2u const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(3f,  array3f const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3i,  array3i const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3ui, array3u const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(4f,  array4f const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4i,  array4i const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4ui, array4u const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(1fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4fv, f32* v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(1iv, i32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2iv, i32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3iv, i32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4iv, i32* v)

	CREATE_UNIFORM_FUNCTION_DEFINITON(1uiv, u32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2uiv, u32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3uiv, u32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4uiv, u32* v)

	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2fv,   std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3fv,   std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4fv,   std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x3fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x2fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x4fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x2fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x4fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x3fv, std::vector<f32> const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2fv,   f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3fv,   f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4fv,   f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x3fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x2fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x4fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x2fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x4fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x3fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4fv, math::mat4f const& v)
#undef CREATE_UNIFORM_FUNCTION_DEFINITON
} Program;