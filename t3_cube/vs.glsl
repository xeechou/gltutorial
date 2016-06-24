#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
out vec3 outColor;
out vec2 TexCoord;

uniform mat4 transform;

void main(){
     
    gl_Position = transform * vec4(position, 1.0f);		
    outColor = color;
    TexCoord = vec2(texCoord.x, 1.0-texCoord.y);
}

