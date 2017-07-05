#version 330 core

in vec3 TexCoords; // direction vector representing a 3D texture coordinate
uniform samplerCube cubemap; // cubemap texture sampler
out vec4 FragColor;

void main()
{
	FragColor = texture(cubemap, TexCoords);
}  
