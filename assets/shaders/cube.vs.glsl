#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;
out vec3 fPosition;
out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fEyePos;
out vec3 fLightDir;

struct LightSource {
    int type;
    vec3 position;
    vec3 normal;
    vec3 color;
    float intensity;
    vec3 _pad1;
};

layout (std140) uniform LightBlock {
    LightSource uLights[12];
};

uniform MatrixBlock {
    mat4 uProj;
    mat4 uView;
};

uniform mat4 uModel;

void main()
{
    gl_Position = uProj * uView * uModel * vec4(vPosition, 1.0);
    fPosition = (uView * uModel * vec4(vPosition, 1.0)).xyz;
    fTexCoord = vTexCoord;
    fNormal = vNormal;
    fEyePos = -(uView * uModel * vec4(vPosition, 1.0)).xyz;
}
