#version 430

out vec4 FragColor;

uniform sampler2D uFramebuffer;
uniform sampler2D uNormalbuffer;
uniform sampler2D uDepthbuffer;
uniform sampler2D uDepth;
uniform vec2 uScreenSize;

float linearDepth(vec2 coord) {
  //float depth = texture(uDepthbuffer, coord).r;
  //return 2.0 * 0.1 * 200.0 / (200.1 - (2.0 * depth - 1.0) * (199.9));
  return texture(uDepth, coord).r * 200.0;
}

void main() {

  // If a depth step change has been detected
  bool depthStepChange = false;

  // Current pixel being rendered normalized from [0..1]
  vec2 currentPixel = vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y);

  vec3 currentColor = floor(texture(uFramebuffer, currentPixel).rgb * 3) / 3;
  //vec3 currentColor = floor(texture(uNormalbuffer, currentPixel).rgb * 4) / 4;
  //vec3 currentBrightness = texture(uFramebuffer, currentPixel).rgb;
  ////vec3 currentColor = vec3(texture(uFramebuffer, currentPixel).r);
  //currentBrightness = floor(currentBrightness * 3) / 3;
  //currentColor *= currentBrightness;

  // Horizontal normal gradient
  float horizontalDeltaNormal = distance(
    texture(uNormalbuffer, vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)).rgb,
    texture(uNormalbuffer, vec2((gl_FragCoord.x + 1) / uScreenSize.x, currentPixel.y)).rgb
  );

  // Vertical normal gradient
  float verticalDeltaNormal = distance(
    texture(uNormalbuffer, vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)).rgb,
    texture(uNormalbuffer, vec2(currentPixel.x, (gl_FragCoord.y + 1) / uScreenSize.y)).rgb
  );

  // Break in normal detected?
  if (horizontalDeltaNormal > 0.5 || verticalDeltaNormal > 0.5) {

    // Draw contour
    currentColor = vec3(0);

    // Try to detect break in depth
  } else {
    float depth = linearDepth(currentPixel);

    // Left depth step change
    float leftDeltaDepth =
      linearDepth(vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)) -
      depth;

    // Right depth step change
    float rightDeltaDepth = depth -
      linearDepth(vec2((gl_FragCoord.x + 1) / uScreenSize.x, currentPixel.y));

    // Down depth step change
    float downDeltaDepth =
      linearDepth(vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)) -
      depth;

    // Up depth step change
    float upDeltaDepth = depth -
      linearDepth(vec2(currentPixel.x, (gl_FragCoord.y + 1) / uScreenSize.y));

    // Detect a smooth gradient
    if (leftDeltaDepth - rightDeltaDepth > 0.01 * depth) {

      // It is in fact an edge, detect height jump
      if (abs(leftDeltaDepth) > 0.001 * depth || abs(rightDeltaDepth) > 0.001 * depth) {
        currentColor = vec3(0);
      }

      // Same for vertical
    } else if (downDeltaDepth - upDeltaDepth > 0.01 * depth) {
      if (abs(downDeltaDepth) > 0.001 * depth || abs(upDeltaDepth) > 0.001 * depth) {
        currentColor = vec3(0);
      }
    }
  }

  FragColor = vec4(currentColor, 1.0);
}
