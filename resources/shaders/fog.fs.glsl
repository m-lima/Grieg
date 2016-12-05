#version 430

out vec4 FragColor;

uniform sampler2D uFramebuffer;
uniform sampler2D uDepthbuffer;
uniform sampler2D uDepth;
uniform vec2 uScreenSize;

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

const float cap = 20.0f;

float linearDepth() {
  float depth = texture(
    uDepthbuffer,
    vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y)).r;
  return 2.0 * 0.1 * 200.0 / (200.1 - (2.0 * depth - 1.0) * (199.9));
}

// Fetch a color from the frame buffer with the given offsets
vec3 textureOffset(int offsetX, int offsetY) {
  return texture(uFramebuffer,
                 vec2((gl_FragCoord.x + offsetX) / uScreenSize.x,
                 (gl_FragCoord.y + offsetY) / uScreenSize.y)).rgb;
}

void main() {

  vec3 currentColor = textureOffset(0, 0);

  // Map the pixel to distance
  // This generates a rapidly increasing value from a central focal point
  float depth = linearDepth();

  // Capping off the blurring for a tilt-shift effect
  if (depth > cap) {
    depth = cap;
  }

  depth = 1.0 - (depth / cap);

  FragColor = vec4(currentColor * depth, 1.0);
}
