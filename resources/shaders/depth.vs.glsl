#version 430

layout(location = 0) in vec3 vPosition;
out vec3 fPosition;

layout(std430, binding = 0) buffer MatrixBlock {
  mat4 uProj;
  mat4 uView;
};

uniform mat4 uModel;

void main() {
  fPosition = vPosition;
  gl_Position = uProj * uView * uModel * vec4(vPosition, 1.0);
}
