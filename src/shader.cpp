#include <glad/glad.h>
#include "shader.hpp"
#include "file.hpp"




constexpr const char* shaderTypeToString(u32 type)
{
    constexpr std::array<u32,         3> types = {  GL_VERTEX_SHADER,   GL_FRAGMENT_SHADER,   GL_COMPUTE_SHADER  };
    constexpr std::array<const char*, 4> strs  = { "GL_VERTEX_SHADER", "GL_FRAGMENT_SHADER", "GL_COMPUTE_SHADER", "GL_SHADER_UNKOWN" };
    size_t i = 0;
    while(i < 3 && types[i] != type) ++i;
    return strs[i];
}




void Program::bind()   const { glUseProgram(m_id); return; }
void Program::unbind() const { glUseProgram(0);	   return; }
void Program::destroy() 
{
    glDeleteProgram(m_id);
    m_id = DEFAULT32;
    return;
}


bool Program::loadShader(ShaderData& init, BufferData& loadedShader)
{
    i32 successStatus = GL_TRUE;
    i32 length = __scast(i32, loadedShader.size);

    init.id = glCreateShader(init.type);
    glShaderSource(init.id, 1, &loadedShader.data, &length);
    glCompileShader(init.id);
    glGetShaderiv(init.id, GL_COMPILE_STATUS, &successStatus);
    if(!successStatus) {
        glGetShaderInfoLog(init.id, genericErrorLog.size(), &length, genericErrorLog.data());
        printf("Shader [type %s] Error Log [%d Bytes]: %s\n", shaderTypeToString(init.type), length, genericErrorLog);

        glDeleteShader(init.id);
        init.id = DEFAULT32;
    }

    return boolean(successStatus);
}


void Program::createProgram()
{
    i32 successStatus = GL_TRUE;


    m_id = glCreateProgram();
    for(auto& shader: shaders) { 
        glAttachShader(m_id, shader.id); 
    }
    glLinkProgram(m_id);


    glGetProgramiv(m_id, GL_LINK_STATUS, &successStatus);
    if(!successStatus) 
    {
        glGetProgramInfoLog(m_id, sizeof(genericErrorLog), NULL, genericErrorLog.data());
        printf("%s\n", genericErrorLog.data());

        glDeleteProgram(m_id);
        m_id = DEFAULT32;
    }

    /* We do this whether successful or not, we don't need leftover shaders since we linked them already. */
    for(auto& shader: shaders) { 
        glDeleteShader(shader.id); 
        shader.id = DEFAULT32; 
    }
    return;
}


void Program::fromFiles(std::vector<ShaderData> const& shaderInfo)
{
    i32 successStatus = GL_TRUE;
    u32 i = 0;
    BufferData buf = {};


    shaders = shaderInfo;
    sources.resize(shaders.size());
    for(; i < shaders.size() && successStatus; ++i)
    {
        buf.size = 0;
        buf.data = nullptr;


        /* we use the text obtained from the shader source for reloading the shader-program on the fly  */
        loadFile(shaders[i].filepath, &buf.size, nullptr );
        sources[i].resize(buf.size);
        loadFile(shaders[i].filepath, &buf.size, sources[i].data());
        

        buf.data = sources[i].data();
        successStatus = successStatus && loadShader(shaders[i], buf);
    }


    if(!successStatus) 
    {
        debug_message("Shader Program Couldn't be created");
        for(u32 s = 0; s < i; ++s) {
            glDeleteShader(shaders[s].id);
            shaders[s].id = DEFAULT32;
        } 
        return;
    }

    createProgram();
    return;
}


void Program::fromBuffers(std::vector<loadedShader> const& buffers)
{
    shaders.reserve(buffers.size());
    i32 successStatus = GL_TRUE;
    u32 i = 0;
    BufferData buf = {};
    

    for(i = 0; i < buffers.size() && successStatus; ++i)
    {
        shaders[i].filepath = nullptr;
        shaders[i].type 	= buffers[i].second;

        buf = buffers[i].first;
        successStatus = successStatus && loadShader(shaders[i], buf);
    }


    if(!successStatus) 
    {
        debug_message("Shader Program Couldn't be created");
        for(u32 s = 0; s < i; ++s) {
            glDeleteShader(shaders[s].id);
            shaders[s].id = DEFAULT32;
        } 
        return;
    }


    createProgram();
    return;
}



#define CREATE_UNIFORM_FUNCTION_IMPL(TypeSpecifier, arg0, ...) \
[[maybe_unused]] void Program::uniform##TypeSpecifier( \
	std::string_view const& name, \
	arg0 \
	) { \
		glUniform##TypeSpecifier( glGetUniformLocation(m_id, name.data()), __VA_ARGS__); \
	} \


CREATE_UNIFORM_FUNCTION_IMPL(1f,  f32 v, v);
CREATE_UNIFORM_FUNCTION_IMPL(1i,  i32 v, v);
CREATE_UNIFORM_FUNCTION_IMPL(1ui, u32 v, v);

CREATE_UNIFORM_FUNCTION_IMPL(2f,  array2f const& v, v[0], v[1]			 );
CREATE_UNIFORM_FUNCTION_IMPL(2i,  array2i const& v, v[0], v[1]			 );
CREATE_UNIFORM_FUNCTION_IMPL(2ui, array2u const& v, v[0], v[1]			 );

CREATE_UNIFORM_FUNCTION_IMPL(3f,  array3f const& v, v[0], v[1], v[2]		 );
CREATE_UNIFORM_FUNCTION_IMPL(3i,  array3i const& v, v[0], v[1], v[2]		 );
CREATE_UNIFORM_FUNCTION_IMPL(3ui, array3u const& v, v[0], v[1], v[2]		 );

CREATE_UNIFORM_FUNCTION_IMPL(4f,  array4f const& v, v[0], v[1], v[2], v[3]);
CREATE_UNIFORM_FUNCTION_IMPL(4i,  array4i const& v, v[0], v[1], v[2], v[3]);
CREATE_UNIFORM_FUNCTION_IMPL(4ui, array4u const& v, v[0], v[1], v[2], v[3]);

CREATE_UNIFORM_FUNCTION_IMPL(1fv, f32* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(2fv, f32* v, 2, v);
CREATE_UNIFORM_FUNCTION_IMPL(3fv, f32* v, 3, v);
CREATE_UNIFORM_FUNCTION_IMPL(4fv, f32* v, 4, v);

CREATE_UNIFORM_FUNCTION_IMPL(1iv, i32* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(2iv, i32* v, 2, v);
CREATE_UNIFORM_FUNCTION_IMPL(3iv, i32* v, 3, v);
CREATE_UNIFORM_FUNCTION_IMPL(4iv, i32* v, 4, v);

CREATE_UNIFORM_FUNCTION_IMPL(1uiv, u32* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(2uiv, u32* v, 2, v);
CREATE_UNIFORM_FUNCTION_IMPL(3uiv, u32* v, 3, v);
CREATE_UNIFORM_FUNCTION_IMPL(4uiv, u32* v, 4, v);

CREATE_UNIFORM_FUNCTION_IMPL(Matrix2fv,   std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3fv,   std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4fv,   std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x3fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x2fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x4fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x2fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x4fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x3fv, std::vector<f32> const& v, 1, false, v.data());

CREATE_UNIFORM_FUNCTION_IMPL(Matrix2fv,   f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3fv,   f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4fv,   f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x3fv, f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x2fv, f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x4fv, f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x2fv, f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x4fv, f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x3fv, f32* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4fv, math::mat4f const& v, 1, false, v.begin());


#undef CREATE_UNIFORM_FUNCTION_IMPL