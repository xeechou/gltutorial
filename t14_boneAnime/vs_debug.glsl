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
out vec3 bone_color;

uniform int totalbone;
const int maxNbone = 100;
uniform mat4 bone_array[maxNbone];
uniform mat4 MVP;
uniform mat4 model;

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	//get the raw rgb value.
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	//mix is the saturate process, if c.y = 0. it is gonna be all white
	//then getting the
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//ratio is something like 0-1
vec3 hue2rgb(float r)
{
	return hsv2rgb(vec3(r, 1.0, 0.5));
}

void main(void)
{
	vec4 v = vec4(position, 1.0);
	bone_color = hue2rgb(bw0.x / totalbone) * bw0.y +
		hue2rgb(bw1.x / totalbone) * bw1.y +
		hue2rgb(bw2.x / totalbone) * bw2.y +
		hue2rgb(bw3.x / totalbone) * bw3.y;

	gl_Position = MVP * v;
	fragPos = vec3(model * v);
	TexCoords = texCoords;
}
)"
