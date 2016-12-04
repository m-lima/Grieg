#version 430

const float size = 100.0f;
const vec3 vtx[] = {
  vec3(-size,  size, -size),
  vec3(-size, -size, -size),
  vec3(size, -size, -size),
  vec3(size, -size, -size),
  vec3(size,  size, -size),
  vec3(-size,  size, -size),

  vec3(-size, -size,  size),
  vec3(-size, -size, -size),
  vec3(-size,  size, -size),
  vec3(-size,  size, -size),
  vec3(-size,  size,  size),
  vec3(-size, -size,  size),

  vec3(size, -size, -size),
  vec3(size, -size,  size),
  vec3(size,  size,  size),
  vec3(size,  size,  size),
  vec3(size,  size, -size),
  vec3(size, -size, -size),

  vec3(-size, -size,  size),
  vec3(-size,  size,  size),
  vec3(size,  size,  size),
  vec3(size,  size,  size),
  vec3(size, -size,  size),
  vec3(-size, -size,  size),

  vec3(-size,  size, -size),
  vec3(size,  size, -size),
  vec3(size,  size,  size),
  vec3(size,  size,  size),
  vec3(-size,  size,  size),
  vec3(-size,  size, -size),

  vec3(-size, -size, -size),
  vec3(-size, -size,  size),
  vec3(size, -size, -size),
  vec3(size, -size, -size),
  vec3(-size, -size,  size),
  vec3(size, -size,  size),
};

out vec3 fTexCoords;

layout(std430, binding = 0) buffer MatrixBlock {
  mat4 uProj;
  mat4 uView;
};

void main() {
  fTexCoords = vtx[gl_VertexID];
  mat4 view = uView;
  view[3] = vec4(0, 0, 0, 1);
  gl_Position = uProj * view * vec4(fTexCoords, 1.0);
}
