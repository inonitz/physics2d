#version 460 core
layout(location = 0) in  vec2 texCoords;
layout(location = 0) out vec4 finalCol;

uniform sampler2D texData;
uniform vec4      texelMultiply;

void main()
{
	vec4 ourColor = vec4(1.0f, 0.0f, 0.5f, 1.0f);
	vec4 intrmd   = texture(texData, texCoords) * texelMultiply;
	finalCol      = intrmd;
}


