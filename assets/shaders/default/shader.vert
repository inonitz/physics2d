#version 460 core
in  vec4 PosTex;
out vec2 TexCoords;


uniform mat4 modelMatrix;


void main()
{
	gl_Position = modelMatrix * vec4(PosTex.xy, 0.0f, 1.0f);
	TexCoords   = PosTex.zw;
}