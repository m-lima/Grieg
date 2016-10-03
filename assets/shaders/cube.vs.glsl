#version 430

layout(location=0) in vec3 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec3 normal;
out vec3 vertexOut;
out vec2 fTexCoord;
out vec3 fNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vertexOut = position;
    gl_Position = projection * view * model * vec4(vertexOut, 1.0);
    fTexCoord = texCoord;
    fNormal = normal;
}
