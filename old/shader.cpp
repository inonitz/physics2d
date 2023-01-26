#include "shader.hpp"
#include "file.hpp"




debugnobr(
    static char genericErrorLog[512];
);

// static constexpr std::array<const char*, 3> to_string = {
//     "GL_VERTEX_SHADER",
//     "GL_FRAGMENT_SHADER",
//     "GL_COMPUTE_SHADER",
// };

static constexpr std::array<u32, 3> to_gl_enum = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_COMPUTE_SHADER
};




Shader::Shader(std::string_view const& path, u8 type)
{
    ShaderMetadata meta    = loadShader(path, type);
    i32            success = static_cast<i32>(meta.srcLength);


    id = glCreateShader(to_gl_enum[type]);
    debugnobr(srcData = meta);
    
    glShaderSource(id, 1, (char**)&meta.srcPointer, &success); /* Specify the shader source text to OpenGL */
    afree_t(meta.srcPointer);
    debugnobr(srcData.srcPointer = nullptr);


    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    ifcrashdo(!success, { /* Safety Check. More info in debug mode. */
        glGetShaderInfoLog(id, sizeof(genericErrorLog), NULL, genericErrorLog);
        printf("%s\n", genericErrorLog);
    });
    
    return;
}



void ShaderProgram::create()
{
    shaderID.reserve(4); // program, vertex, frag, compute
    shaderID.push_back(glCreateProgram());

    debugnobr(shaderDebug.reserve(4));
    return;
}


void ShaderProgram::destroy()
{
    glDeleteProgram(shaderID[0]);
    return;
}


ShaderProgram& ShaderProgram::addShader(Shader const& shader)
{
    shaderID.push_back(shader.id);
    debugnobr(shaderDebug.push_back(shader.srcData));

    return *this;
}


ShaderProgram& ShaderProgram::addShader(std::string_view const& path, u8 shaderType)
{ 
    Shader tmp{path, shaderType};
    return addShader(tmp);
}


void ShaderProgram::finalize()
{
    i32 success;

    for(size_t i = 1; i < shaderID.size(); ++i) {
        glAttachShader(shaderID[0], shaderID[i]); /* ShaderID[0] is the ID of the shader Program. */
    }

    /* Link shader program and check for errors. Delete shaders when finished. */
	glLinkProgram(shaderID[0]);
	glGetProgramiv(shaderID[0], GL_LINK_STATUS, &success);
	ifcrashdo(!success, {
		glGetProgramInfoLog(shaderID[0], sizeof(genericErrorLog), NULL, genericErrorLog);
		printf("%s\n", genericErrorLog);
	});

	for(size_t i = 1; i < shaderID.size(); ++i) {
		glDeleteShader(shaderID[i]);
	}


	return;
}




ShaderMetadata loadShader(std::string_view const& path, u8 stype) 
{
	ShaderMetadata mdata { stype, 0, nullptr };


	loadFile(path.cbegin(), (size_t*)&mdata.srcLength, (char*)mdata.srcPointer); /* Get Necessary Data in mdata */
	
    mdata.srcPointer = amalloc_t(u8, mdata.srcLength, CACHE_LINE_BYTES);
	ifcrash(!boolean(mdata.srcPointer));
	
    loadFile(path.cbegin(), (size_t*)&mdata.srcLength, (char*)mdata.srcPointer);
	return mdata;
}