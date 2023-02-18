#version 460 core
layout(location = 0) in  vec2 texCoords;
layout(location = 0) out vec4 finalCol;

uniform sampler2D texData;


/*
layout(std460) buffer testing_ssbo {
	vec2i textureDimensions;
	vec4[] textureColors;
};
*/


void main()
{
	vec4 intrmd = texture(texData, texCoords);
	finalCol    = intrmd;
}


/*
	Figure out how to test the SSBO struct,
	probably finish integrating the compute shader into the project,
	and from there (if all goes according to plan) use the SSBO as part of the rendering process,
	so that there'll be a visual difference that will make me notice that it worked.
*/


