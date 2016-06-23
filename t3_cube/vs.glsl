#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
out vec3 outColor;
out vec2 TexCoord;



void main(){
    gl_Position = vec4(position, 1.0f);		
    outColor = color;
    TexCoord = texCoord;
}

