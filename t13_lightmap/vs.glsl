#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in mat4 instancesMat;

uniform mat4 MVP;
uniform mat4 lightMat;
uniform vec3 lightPos;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
	vec4 FragPosLS;
	vec4 FragPosCS;
} vs_out;

void main()
{
	vs_out.FragPos = vec3(instancesMat * vec4(position, 1.0f));
	vs_out.Normal  = transpose(inverse(mat3(instancesMat))) * normal;
	vs_out.TexCoord = texcoord;
	gl_Position =           MVP * vec4(vs_out.FragPos, 1.0);
	vs_out.FragPosLS = lightMat * vec4(vs_out.FragPos, 1.0);
	vs_out.FragPosCS = gl_Position;
}

