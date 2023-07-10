#version 460 core
in  vec2 TexCoords;
out vec4 FragColor;


uniform sampler2D uTexture;


void main()
{  
    FragColor = vec4( texture(uTex, TexCoords).rgb , 1.0);
}


