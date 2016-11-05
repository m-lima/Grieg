#version 430

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec3 vNormal;
out vec3 fNormal;

layout(std430, binding = 0) buffer MatrixBlock {
  mat4 uProj;
  mat4 uView;
};

uniform mat4 uModel;

void main() {
  gl_Position = uProj * uView * uModel * vec4(vPosition, 1.0);
  fNormal = normalize(vNormal);
}
