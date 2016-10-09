#version 430

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal; //Already normalized
out vec4 FragColor;

uniform vec3 lightPos; //Assumed to be normalized to avoid per-pixel normalization

uniform vec3 sunPos; // Sun position in global coordinates

uniform sampler2D skybox;

uniform sampler2D uTexture;

void main() {
    float flashlight = dot(fNormal, lightPos) * 0.5;
    vec3 flashlightColor = vec3(flashlight);

    float sunLight = dot(fNormal, sunPos);
    vec3 sunColor = vec3(sunLight);

    vec4 texel = texture(uTexture, fTexCoord);
    FragColor = vec4(texel.xyz * clamp((sunColor + flashlightColor), 0.3, 1.0), texel.w);
}
