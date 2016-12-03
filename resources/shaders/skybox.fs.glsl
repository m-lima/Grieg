#version 430

in vec3 fTexCoords;
out vec4 FragColor;

uniform samplerCube uCubeTexture;

void main() {
  FragColor = texture(uCubeTexture, fTexCoords);
}
