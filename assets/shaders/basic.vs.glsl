#version 430

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;
out vec3 fPosition;
out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fEyePos;

layout(std430, binding = 0) buffer MatrixBlock {
  mat4 uProj;
  mat4 uView;
};

uniform mat4 uModel;

void main() {
  fPosition = vPosition;
  gl_Position = uProj * uView * uModel * vec4(vPosition, 1.0);
  fTexCoord = vTexCoord;
  fNormal = normalize((uModel * vec4(vNormal, 1.0)).xyz);
  fEyePos = (inverse(uView) * inverse(uModel) * vec4(0.0, 0.0, 5.0, 1.0)).xyz;
}
