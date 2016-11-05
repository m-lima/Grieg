#version 430

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal; //Already normalized
in vec3 fEyePos;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform int uHaveTexture;

uniform sampler2D uFramebuffer;
uniform vec2 uScreenSize;

uniform vec3 uAmbientLight;

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

void main() {

  vec2 currentPixel = vec2(gl_FragCoord.x / uScreenSize.x, gl_FragCoord.y / uScreenSize.y);

  vec3 currentColor = texture(uFramebuffer, currentPixel).rgb;

  if (currentColor != texture(uFramebuffer, vec2((gl_FragCoord.x - 1) / uScreenSize.x, currentPixel.y)).rgb
      || currentColor != texture(uFramebuffer, vec2(currentPixel.x, (gl_FragCoord.y - 1) / uScreenSize.y)).rgb) {
    currentColor = vec3(0);
  }

  FragColor = vec4(currentColor, 1.0);
}
