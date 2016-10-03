#version 430

layout(location=0) in vec3 vPosition;
out vec3 fPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    fPosition = vPosition;
    gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
