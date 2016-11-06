#version 430

in vec3 fPosition;

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

// Fetch a color from the frame buffer with the given offsets
vec3 textureOffset(int offsetX, int offsetY) {
  return texture(uFramebuffer, vec2((gl_FragCoord.x + offsetX) / uScreenSize.x, (gl_FragCoord.y + offsetY) / uScreenSize.y)).rgb;
}

void main() {

  vec3 currentColor = vec3(0);

  float depth = texture(uDepthbuffer, vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y)).r;
  //depth = (2.0 * NEAR) / (FAR + NEAR - depth * (FAR - NEAR));

  //depth = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
  //depth /= gl_FragCoord.w;
  //depth = depth * 0.5 + 0.5;

  //depth = ((2.0 * depth - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near));

  //depth = (NEAR * depth) / (FAR - depth * (FAR - NEAR));

  //depth = (2.0 * gl_DepthRange.near) / (gl_DepthRange.far + gl_DepthRange.near - depth * (gl_DepthRange.far - gl_DepthRange.near));

  depth = min(1, max((distance(vec3(0), fPosition) / 25) - 0.9, 0)) - 1;
  float factor;
  float weights = 0;

  for (int i = -3; i < 4; i++) {
    for (int j = -3; j < 4; j++) {

      // Fake gaussian - an approxiation I tested on Octave
      // should work close enough, though much faster
      //
      // ** IF WE INCREASE THE KERNEL, WE HAVE TO REMEMBER
      // ** TO EXPAND THE BOUNDS OF THIS FORMULA
      //factor = 1 - pow(i / 10, 2) - pow(j / 10, 2);
      //float iFactor = i != 0 ? 1 - i*i / 10 : 1;
      //float jFactor = j != 0 ? 1 - j*j / 10: 1;
      //factor = iFactor * jFactor;
      factor = exp(depth*pow(i, 2)) * exp(depth*pow(j, 2));
      factor *= factor;
      //factor = pow((depth), abs(i)) * pow((depth), abs(j));
      weights += factor;

      // Factor in the depth (it varies within [-1..1]
      //factor *= depth;

      //factor = 1 - factor;

      // Apply current weighted color into final color
      currentColor += textureOffset(i, j) * factor;
    }
  }

  // Finalize the weighted average
  currentColor /= weights;

  //FragColor = vec4(vec3(depth), 1.0);
  FragColor = vec4(currentColor, 1.0);
}
