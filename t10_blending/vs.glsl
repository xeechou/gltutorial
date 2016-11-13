#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 offset;

out vec2 TexCoords;
out vec3 fragPos;
out vec3 Normal;
out vec3 Color;
uniform mat4 MVP;
uniform mat4 model;

//uniform vec3 offsets[10];

void main(void)
{
//	vec3 offset = offsets[gl_InstanceID];
	gl_Position = MVP * vec4(position + offset, 1.0f);
	Normal = normal;
	fragPos = vec3(model * vec4(position + offset, 1.0f));
	TexCoords = vec2(texCoords.x, 1-texCoords.y);
	Color = offset;
}
