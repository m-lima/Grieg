#version 430

out vec4 FragColor;

uniform sampler2D uFramebuffer;
uniform sampler2D uDepthbuffer;
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

float linearDepth(vec2 coord) {
  return texture(uDepthbuffer, coord).r;
}

void main() {

  // If a depth step change has been detected
  bool depthStepChange = false;

  // Current pixel being rendered normalized from [0..1]
  vec2 currentPixel = vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y);

  vec3 currentColor = texture(uFramebuffer, currentPixel).rgb;

  // Horizontal normal gradient
  float horizontalDeltaNormal = distance(
    texture(uFramebuffer, vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)).rgb,
    texture(uFramebuffer, vec2((gl_FragCoord.x + 1) / uScreenSize.x, currentPixel.y)).rgb
  );

  // Vertical normal gradient
  float verticalDeltaNormal = distance(
    texture(uFramebuffer, vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)).rgb,
    texture(uFramebuffer, vec2(currentPixel.x, (gl_FragCoord.y + 1) / uScreenSize.y)).rgb
  );

  // Break in normal detected?
  if (horizontalDeltaNormal > 0.5 || verticalDeltaNormal > 0.5) {

    // Draw contour
    currentColor = vec3(0);

  // Try to detect break in depth
  } else {
  
    // Left depth step change
    float leftDeltaDepth =
      linearDepth(vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)) -
      linearDepth(currentPixel);

    // Right depth step change
    float rightDeltaDepth = linearDepth(currentPixel) -
      linearDepth(vec2((gl_FragCoord.x + 1) / uScreenSize.x, currentPixel.y));

    // Down depth step change
    float downDeltaDepth =
      linearDepth(vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)) -
      linearDepth(currentPixel);

    // Up depth step change
    float upDeltaDepth = linearDepth(currentPixel) -
      linearDepth(vec2(currentPixel.x, (gl_FragCoord.y + 1) / uScreenSize.y));

    // Detect a smooth gradient
    if (leftDeltaDepth - rightDeltaDepth > 0.001) {

      // It is in fact an edge, detect height jump
      if (abs(leftDeltaDepth) > 0.0001) {
        currentColor = vec3(0);
      }

    // Same for vertical
    } else if (downDeltaDepth - upDeltaDepth > 0.001) {
      if (abs(downDeltaDepth) > 0.0001) {
        currentColor = vec3(0);
      }
    }
  }

  FragColor = vec4(currentColor, 1.0);
}
