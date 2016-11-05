#version 430

out vec4 FragColor;

uniform sampler2D uFramebuffer;
uniform sampler2D uDepthbuffer;
uniform vec2 uScreenSize;

const float NEAR = 1; // From Trackball.cc projection
const float FAR = 20; // From Trackball.cc projection

struct LightSource {
  int type;
  vec3 direction;
  vec3 color;
  vec3 position;
  float specularIndex;
  float aperture;
  float intensity;
};

layout(std430, binding = 1) buffer LightBlock {
  LightSource uLights[12];
};

layout(std430, binding = 2) buffer MaterialBlock {
  vec3 uAmbient;
  vec3 uDiffuse;
  vec3 uSpecular;
};

vec3 textureOffset(int offsetX, int offsetY) {
  return texture(uFramebuffer, vec2((gl_FragCoord.x + offsetX) / uScreenSize.x, (gl_FragCoord.y + offsetY) / uScreenSize.y)).rgb;
}

void main() {

  vec3 currentColor = vec3(0);

  float depth = texture(uDepthbuffer, vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y)).r;
  //depth = (2.0 * NEAR) / (FAR + NEAR - depth * (FAR - NEAR));
  depth = (NEAR * depth) / (FAR - depth * (FAR - NEAR));
  //depth = (2.0 * gl_DepthRange.near) / (gl_DepthRange.far + gl_DepthRange.near - depth * (gl_DepthRange.far - gl_DepthRange.near));
  float factor = 0;

  for (int i = -2; i < 3; i++) {
    for (int j = -2; j < 3; j++) {
      factor = 1 - pow(i / 10, 2) * 10 - pow(j / 10, 2) * 10;
      currentColor += textureOffset(i, j) * factor;
    }
  }
  currentColor /= 25;

  FragColor = vec4(vec3(depth), 1.0);
  //FragColor = vec4(currentColor, 1.0);
}
