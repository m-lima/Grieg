#version 430

in vec3 position; 
out vec3 vertexOut;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 model;

void main()
{
    vertexOut = position;    
	gl_Position = projection * view * vec4(vertexOut, 1.0);
}
