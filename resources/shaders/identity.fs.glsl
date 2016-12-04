#version 430

out vec4 FragColor;

uniform sampler2D uFramebuffer;
uniform sampler2D uNormalbuffer;
uniform sampler2D uDepthbuffer;
uniform vec2 uScreenSize;

void main() {
  vec2 currentPixel = vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y);

  FragColor = vec4(texture(uFramebuffer, currentPixel).rgb, 1.0);
}
