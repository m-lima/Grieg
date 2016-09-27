#version 430

in vec3 vertexOut;
in vec3 fNormal; //Already normalized
out vec4 fragColor;

uniform vec3 lightPos; //Assumed to be normalized to avoid per-pixel normalization
uniform mat4 view;

uniform vec3 sunPos; // Sun position in global coordinates

void main() {       
	float flashlight = dot(fNormal, lightPos) * 0.5;
	vec3 flashlightColor = vec3(flashlight);
 
	float sunLight = dot(fNormal, sunPos);
	vec3 sunColor = vec3(sunLight);

    fragColor = vec4(sunColor + flashlightColor, 1.0);
}