#version 430

in vec3 fNormal; //Already normalized
out vec4 FragColor;

struct LightSource {
    int type;
    vec3 direction;
    vec3 color;
    vec3 position;
    float specularIndex;
    float aperture;
    float intensity;
};

layout(std430, binding = 1) buffer LightBlock{
    LightSource uLights[12];
};

layout(std430, binding = 2) buffer MaterialBlock {
    vec3 uAmbient;
    vec3 uDiffuse;
    vec3 uSpecular;
};

void main() {
  FragColor = vec4(fNormal / 2 + 0.5, 1.0);
}