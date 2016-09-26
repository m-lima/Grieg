#version 430

in vec3 position;
in vec3 normal;
out vec3 vertexOut;
out vec3 fNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vertexOut = position;    
	gl_Position = projection * view * model * vec4(vertexOut, 1.0);
    fNormal = normalize(normal);
}
