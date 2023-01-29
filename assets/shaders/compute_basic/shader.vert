#version 460 core
layout(location = 0) in  vec4 vPos;
layout(location = 1) in  vec2 vTexCoord;
layout(location = 0) out vec2 texCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vPos;
	texCoords   = vTexCoord;
}