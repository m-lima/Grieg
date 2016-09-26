#version 430

in vec3 vertexOut;
in vec3 fNormal;
out vec4 fragColor;

void main() {       
    fragColor = vec4(normalize(fNormal), 1.0);
}