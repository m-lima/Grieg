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

const float focus = 5.0f;
const float cap = 2.0f;

// Fetch a color from the frame buffer with the given offsets
vec3 textureOffset(int offsetX, int offsetY) {
  return texture(uFramebuffer,
                 vec2((gl_FragCoord.x + offsetX) / uScreenSize.x,
                 (gl_FragCoord.y + offsetY) / uScreenSize.y)).rgb;
}

void main() {

  vec3 currentColor = vec3(0);
  float weights = 0;

  // Map the pixel to distance
  // This generates a rapidly increasing value from a central focal point
  float depth = abs(texture(uDepth, gl_FragCoord.xy).r - focus);

  // Capping off the blurring for a tilt-shift effect
  //if (depth > cap) {
  //  depth = cap;
  //}

  // Distance can only make matters worse
  //depth += 1.0f;

  float factor;
  for (int i = -3; i < 4; i++) {
    for (int j = -3; j < 4; j++) {

      // Fake gaussian - an approxiation I tested on Octave
      // should work close enough, though much faster
      // ** Very sensitive!! Only makes since if doing tilt-shifting
      if (i == 0 || j == 0) {
        factor = depth;
      } else {
        factor = depth / abs(i);
        factor *= depth / abs(j);
      }

      // Store the current factor into the overall weight
      weights += factor;

      // Apply current weighted color into final color
      currentColor += textureOffset(i, j) * factor;
    }
  }

  // Finalize the weighted average
  currentColor /= weights;

  //FragColor = vec4(currentColor, 1.0);
  FragColor = vec4(vec3(depth), 1.0);
}
