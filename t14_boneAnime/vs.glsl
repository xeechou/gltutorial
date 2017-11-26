R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec2 bw0;
layout (location = 4) in vec2 bw1;
layout (location = 5) in vec2 bw2;
layout (location = 6) in vec2 bw3;

out vec2 TexCoords;
out vec3 fragPos;
out vec3 Normal;

//we couldn't really use a dynamic size of uniform array, since uniform size has
//limitations

const int maxNbone = 100;
uniform mat4 MVP;
uniform mat4 model;
uniform mat4 bone_array[maxNbone];

void main(void)
{
	vec4 v = vec4(position, 1.0);
	vec4 n = vec4(normal, 1.0);
	vec4 newVertex;
	vec4 newNormal;

	//updating vertex
	newVertex = (v * bone_array[int(bw0.x)]) * bw0.y +
		    (v * bone_array[int(bw1.x)]) * bw1.y +
		    (v * bone_array[int(bw2.x)]) * bw2.y +
		    (v * bone_array[int(bw3.x)]) * bw3.y;
	//updating normal
	newNormal = (n * bone_array[int(bw0.x)]) * bw0.y +
		    (n * bone_array[int(bw1.x)]) * bw1.y +
		    (n * bone_array[int(bw2.x)]) * bw2.y +
		    (n * bone_array[int(bw3.x)]) * bw3.y;
	gl_Position = MVP * newVertex;
	Normal = vec3(newNormal);
	fragPos = vec3(model * newVertex);
	TexCoords = texCoords;
}

)"
