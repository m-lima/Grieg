#version 430

in vec3 position; 
out vec3 vertexOut;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 model;

void main()
{
    vertexOut = position;    
    //gl_Position = projection * view * model * vec4((vertexOut - 0.5), 1.0);
	gl_Position = projection * view * vec4((vertexOut - 0.5), 1.0);
}
