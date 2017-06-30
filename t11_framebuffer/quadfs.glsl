#version 330 core

in  vec2 TexCoords;
//in  vec3 fragPos;
out vec4 color;

uniform sampler2D ST;

void main(void)
{
//	color = vec4(vec3(texture(ST, TexCoords)), 1.0);
	color = vec4(vec3(1.0 - texture(ST, TexCoords)), 1.0);
}
