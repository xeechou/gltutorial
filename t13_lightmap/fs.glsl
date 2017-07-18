#version 330 core

out vec4 FragColor;

uniform vec3 lightpos;

void main(void)
{
	if (gl_FragCoord.x < 400)
		FragColor = vec4(1.0, 0.0, 0.0, 1.0f);
	else
		FragColor = vec4(0.0, 1.0, 0.0, 1.0f);
}
