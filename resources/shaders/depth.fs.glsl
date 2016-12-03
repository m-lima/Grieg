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

// Fetch a color from the frame buffer with the given offsets
vec3 textureOffset(int offsetX, int offsetY) {
  return texture(uFramebuffer, vec2((gl_FragCoord.x + offsetX) / uScreenSize.x, (gl_FragCoord.y + offsetY) / uScreenSize.y)).rgb;
}

void main() {

  vec3 currentColor = vec3(0);
  float weights = 0;

  // Map the pixel to distance
  // This generates a rapidly increasing value from a central
  // focal point. Then it is capped to [0..1] and shifted to [-1..0]
  float depth = texture(uDepth, gl_FragCoord.xy).r;

  float factor;
  for (int i = -3; i < 4; i++) {
    for (int j = -3; j < 4; j++) {

      // Fake gaussian - an approxiation I tested on Octave
      // should work close enough, though much faster
      // ** Very sensitive!! Only makes since if doing tilt-shifting
      factor = exp(depth*pow(i, 2)) * exp(depth*pow(j, 2));

      // Make it more sensitive for tilt-shifting
      factor = pow(factor, 8);

      // Store the current factor into the overall weight
      weights += factor;

      // Apply current weighted color into final color
      currentColor += textureOffset(i, j) * factor;
    }
  }

  // Finalize the weighted average
  currentColor /= weights;

  FragColor = vec4(currentColor, 1.0);
}
