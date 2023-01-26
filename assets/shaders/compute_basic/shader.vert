#version 460 core
layout(location = 0) in  vec4 vPos;
layout(location = 1) in  vec2 vTexCoord;
layout(location = 0) out vec2 texCoords;


void main()
{
	gl_Position = vPos;
	texCoords   = vTexCoord;
}