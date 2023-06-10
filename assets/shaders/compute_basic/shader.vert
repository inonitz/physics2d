#version 460 core
layout(location = 0) in  vec3 vPos;
layout(location = 1) in  vec2 vTexCoord;
layout(location = 0) out vec2 TexCoords;


// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;


void main()
{
	// gl_Position = projection * view * model * vPos;
	gl_Position = vec4(vPos, 1.0f);
	TexCoords   = vTexCoord;
}