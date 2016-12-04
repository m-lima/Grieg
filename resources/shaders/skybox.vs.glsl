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

uniform mat4 uPV;

void main() {
  fTexCoords = vtx[gl_VertexID];
  gl_Position = uPV * vec4(fTexCoords, 1.0);
}
