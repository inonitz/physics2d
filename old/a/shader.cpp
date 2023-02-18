#include "shader.hpp"
#include "file.hpp"
#include <array>
#include <vector>




namespace OpenGL {


static char genericErrorLog[1024]; /* dont use multi-threaded please :) */
static alignsz(CACHE_LINE_BYTES) char   genericShaderBuffer[32 * KB] = {};
static                           size_t genericShaderBufferPointer   = 0;


__force_inline void* alloc_local(size_t amount)
{
    if(unlikely( genericShaderBufferPointer >= sizeof(genericShaderBuffer) )) {
        debug_message("couldn't allocate space for shader from local stack, please expand its size.\n");
        return nullptr;
    }

    void* out = &genericShaderBuffer[genericShaderBufferPointer];
    genericShaderBufferPointer += amount;


    return out;
}


__force_inline void free_local(size_t amount) {
    if(amount > genericShaderBufferPointer) {
        debug_message("Check your arguments to Shader.load() !!! ");
        return;
    }


    genericShaderBufferPointer -= amount;
    return;
}




constexpr const char* shaderTypeToString(u32 type)
{
    constexpr std::array<u32,         3> types = {  GL_VERTEX_SHADER,   GL_FRAGMENT_SHADER,   GL_COMPUTE_SHADER  };
    constexpr std::array<const char*, 4> strs  = { "GL_VERTEX_SHADER", "GL_FRAGMENT_SHADER", "GL_COMPUTE_SHADER", "GL_SHADER_UNKOWN" };
    size_t i = 0;
    while(i < 3 && types[i] != type) ++i;
    return strs[i];
}




template<u32 Type> Shader<Type> makeShader(const char* filepath)
{
    u32    id     = DEFAULT32;
    size_t size   = 0;
    char*  src    = nullptr;
    i32    length = 0;
    i32    status = GL_TRUE;


    /* Load File */
    loadFile(filepath, &size, src);
	src = reinterpret_cast<char*>(alloc_local(size));
	loadFile(filepath, &size, src);
    length = static_cast<i32>(size);


    /* Create Shader */
    id = glCreateShader(Type);
    glShaderSource(id, 1, (const char**)&src, &length);
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	if(!status) {
		glGetShaderInfoLog(id, sizeof(genericErrorLog), &length, genericErrorLog);
    	printf("Shader [type %s] Error Log [%u Bytes]: %s\n", shaderTypeToString(Type), length, genericErrorLog);

        glDeleteShader(id);
        id = DEFAULT32;
    }


    free_local(size);
    return Shader<Type>(id);
}


template<u32 Type> std::unique_ptr<Shader<Type>> Shader<Type>::load(const char* filepath)
{
    /* Load File */
    size_t size = 0;
    char*  src  = nullptr;
    Shader<Type> out;

    loadFile(filepath, &size, src);
	src = reinterpret_cast<char*>(alloc_local(size));
	loadFile(filepath, &size, src);


    out = Shader<Type>::load(src, size);
    free_local(size);
    return out;
}


template<u32 Type> std::unique_ptr<Shader<Type>> Shader<Type>::load(char* buffer, size_t size)
{
    u32 id  = glCreateShader(Type);
    i32 len = static_cast<i32>(size);
    i32 successStatus = GL_TRUE;


    /* Create Shader */
    glShaderSource(id, 1, (const char**)&buffer, &len);
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &successStatus);
	if(!successStatus) {
		glGetShaderInfoLog(id, sizeof(genericErrorLog), &len, genericErrorLog);
    	printf("Shader [type %s] Error Log [%d Bytes]: %s\n", shaderTypeToString(Type), len, genericErrorLog);

        glDeleteShader(id);
        id = DEFAULT32;
    }

    std::vector<char> source(buffer, buffer + size);
    std::unique_ptr<Shader<Type>> out = std::make_unique<Shader<Type>>()
    return std::make_unique<Shader<Type>>(id, );
}




void Program::create(std::vector<Shader<>> const& makeShaders)
{
    i32 status = true;
    u32 i      = 0;


    m_id = DEFAULT32;
    while(status && i < makeShaders.size())
    {
        status = status && makeShaders[i].m_id != DEFAULT32;        
        ++i;
    }
    if(!status) {
        debug_message("Shader Program Couldn't be created");
        return;
    }

    
    m_id = glCreateProgram();
    m_shaders = makeShaders;
    for(auto& shader : makeShaders) {
        glAttachShader(m_id, shader.m_id);
    }
    glLinkProgram(m_id);
    
    
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);;
    if(!status) {
        glGetProgramInfoLog(m_id, sizeof(genericErrorLog), NULL, genericErrorLog);
		printf("%s\n", genericErrorLog);

		glDeleteProgram(m_id);
		m_id = DEFAULT32;
    }


    for(auto& shader : makeShaders) {
		glDeleteShader(shader.m_id);
	}
    return;
}


void Program::destroy() 
{ 
    glDeleteProgram(m_id);
    m_id = DEFAULT32; 
    return; 
}


void Program::reload(std::vector<Shader)
{

}



}