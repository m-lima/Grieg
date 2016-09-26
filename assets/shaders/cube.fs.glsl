#version 430

in vec3 vertexOut;
in vec3 fNormal;
out vec4 fragColor;

//Assumed to be normalized to avoid per-pixel normalization
uniform vec3 lightPos;

void main() {       
	float bright = dot(normalize(fNormal), lightPos);
    fragColor = vec4(bright, bright, bright, 1.0);
}