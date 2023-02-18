#pragma once
#include <glad/glad.h>
#include <vector>
#include "vec.hpp"




struct ShaderStorageBufferObject 
{
    u32 id;
    u32 shaderProgramID;
    u32 shaderBindingPoint; /* a table of 'references' to buffer objects. */
    u32 shaderBlockIndex;   /* I DONT FUCKING KNOW WHY THIS IS IMPORTANT DON'T ASK ME IM SETTING THIS TO 0 */


    void create(
        std::vector<f32>& uploadToGPU,
        u32 programID, 
        u32 bindingPoint,
        u32 blockIndex = 0u
    ) {
        shaderProgramID    = programID;
        shaderBindingPoint = bindingPoint;
        shaderBlockIndex   = blockIndex;
        glCreateBuffers(1, &id);
        glNamedBufferSubData(id, 0, uploadToGPU.size() * sizeof(f32), uploadToGPU.data());


        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
        glShaderStorageBlockBinding(programID, shaderBlockIndex, bindingPoint);
        return;
    }


    /* Still don't know if I should clear the shader Indices so I'm leaving this const for now. */
    void unbind() const {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, shaderBindingPoint, 0);
        glShaderStorageBlockBinding(shaderProgramID, shaderBlockIndex, 0);
        return;
    }


    void bind() const { 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, shaderBindingPoint, id);
        glShaderStorageBlockBinding(shaderProgramID, shaderBlockIndex, shaderBindingPoint);
        return;
    }


    void destroy() { glDeleteBuffers(1, &id); id = DEFAULT32; return; }
};