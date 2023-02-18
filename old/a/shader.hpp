#pragma once
#include "base.hpp"
#include <glad/glad.h>
#include <type_traits>
#include <vector>
#include <memory>




namespace OpenGL {


template<u32 Type = DEFAULT32> struct Shader {
private:
    static constexpr size_t underlying_type = Type;
    std::vector<char> shaderSource;

public:    
    u32 m_id;

    static std::unique_ptr<Shader<Type>> load(const char* filePath);
    static std::unique_ptr<Shader<Type>> load(char* buffer, size_t size); /*  presume buffer from external source is managed out of scope */
};


using VertexShader   = Shader<GL_VERTEX_SHADER  >;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;
using ComputeShader  = Shader<GL_COMPUTE_SHADER >;
template struct Shader<GL_VERTEX_SHADER  >;
template struct Shader<GL_FRAGMENT_SHADER>;
template struct Shader<GL_COMPUTE_SHADER >;


template<u32 Type> Shader<Type> makeShader(const char* filepath);




struct Program {
    u32 m_id;
    std::vector<Shader<>> m_shaders;


    void create(std::vector<Shader<>> const& shadersToLink);
    void destroy();
    void reload();
};




}

u32 reloadShaders(std::vector< std::pair<u32, const char*> > const& shaderTypes) 
{
	static char genericErrorLog[1024];
	size_t size   		  = 0;
	char*  src    		  = nullptr;
	i32    length 		  = 0;
	i32    successStatus  = GL_TRUE;
	std::vector<u32> shaderID;
	u32    			 newProgram = glCreateProgram();


	shaderID.assign(shaderTypes.size(), DEFAULT32);
	for(u32  i = 0; i < shaderID.size() && successStatus; ++i)
	{
		shaderID[i] = glCreateShader(shaderTypes[i].first);

		size = 0;
		src  = nullptr;

		loadFile(shaderTypes[i].second, &size, src);
		src    = amalloc_t(char, size, CACHE_LINE_BYTES);
		loadFile(shaderTypes[i].second, &size, src);
		length = static_cast<i32>(size);


		glShaderSource(shaderID[i], 1, (const char**)&src, &length);
		glCompileShader(shaderID[i]);
		glGetShaderiv(shaderID[i], GL_COMPILE_STATUS, &successStatus);
		if(successStatus) {
			glAttachShader(newProgram, shaderID[i]);
		} else {
			glGetShaderInfoLog(shaderID[i], sizeof(genericErrorLog), &length, genericErrorLog);
        	printf("Shader [type %u] Error Log[%u Bytes]: %s\n", shaderTypes[i], length, genericErrorLog);
		}
		afree_t(src);
	}

	if(!successStatus) {
		for(auto& shader: shaderID) { glDeleteShader(shader); }
		glDeleteProgram(newProgram);
		return DEFAULT32;
	}


	glLinkProgram(newProgram);
	glGetProgramiv(newProgram, GL_LINK_STATUS, &successStatus);
	if(!successStatus) {
		glGetProgramInfoLog(newProgram, sizeof(genericErrorLog), NULL, genericErrorLog);
		printf("%s\n", genericErrorLog);

		glDeleteProgram(newProgram);
		newProgram = DEFAULT32;
	}


	for(auto& shader : shaderID) {
		glDeleteShader(shader);
	}


	return newProgram;
}