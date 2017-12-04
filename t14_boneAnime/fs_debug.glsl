R"(
#version 330 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 fragPos;
in vec3 bone_color;
out vec4 color;

void main(void)
{
	color = vec4(bone_color, 1.0);
}

)"
