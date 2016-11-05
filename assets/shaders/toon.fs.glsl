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

float linearDepth(vec2 coord) {
  float z = texture(uFramebuffer, coord).r;
  return (NEAR * z) / (FAR - z * (FAR - NEAR));
  //return (2.0 * NEAR) / (FAR + NEAR - z * (FAR - NEAR));
  //return (2 * z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
  //return pow(z, 1024.0);
  //return z;
}

void main() {

  vec2 currentPixel = vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y);

  vec3 currentColor = texture(uFramebuffer, currentPixel).rgb;

  float horizontalDeltaNormal = distance(
    texture(uFramebuffer, vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)).rgb,
    texture(uFramebuffer, vec2((gl_FragCoord.x + 1) / uScreenSize.x, currentPixel.y)).rgb
  );
  
  float verticalDeltaNormal = distance(
    texture(uFramebuffer, vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)).rgb,
    texture(uFramebuffer, vec2(currentPixel.x, (gl_FragCoord.y + 1) / uScreenSize.y)).rgb
  );

  float horizontalDeltaDepth = distance(
    texture(uDepthbuffer, vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)).rgb,
    texture(uDepthbuffer, vec2((gl_FragCoord.x + 1) / uScreenSize.x, currentPixel.y)).rgb
  );
  
  float verticalDeltaDepth = distance(
    texture(uDepthbuffer, vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)).rgb,
    texture(uDepthbuffer, vec2(currentPixel.x, (gl_FragCoord.y + 1) / uScreenSize.y)).rgb
  );

  if (horizontalDeltaNormal > 0.5 || verticalDeltaNormal > 0.5 ||
      horizontalDeltaDepth > 0.1 || verticalDeltaDepth > 0.1) {
    currentColor = vec3(0);
  }

  FragColor = vec4(currentColor, 1.0);
}
