R"(

#version 330 core

in vec3 TexCoords; // direction vector representing a 3D texture coordinate
uniform samplerCube ctex; // cubemap texture sampler
out vec4 color;

void main()
{
	color = vec4(vec3(texture(ctex, TexCoords)), 1.0);
}  

)"
