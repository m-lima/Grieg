#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;
out vec3 fPosition;
out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fEyePos;

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
