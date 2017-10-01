R"(
#version 330 core

layout (location = 0) in vec3 aPos;
out vec3 TexCoords;

uniform mat4 pv;
//uniform mat4 view;

void main()
{
	TexCoords = aPos;
	vec4 pos = pv * vec4(aPos, 1.0);
	//trick OpenGL that our skybox is always at maximum distance
	gl_Position = pos.xyww;
}

)"
