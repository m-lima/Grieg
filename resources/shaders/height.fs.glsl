#version 430

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal; //Already normalized
in vec3 fEyePos;
in float fDepth;
out vec4 FragColor;
out vec4 FragNormal;
out float FragDepth;

uniform sampler2D uTexture;
uniform int uHaveTexture;
uniform sampler2D uBump;
uniform int uHaveBump;

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

// vec3 hsv2rgb(vec3 c)
// {
//   vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
//   vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
//   return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
// }

void main()
{
  // float height = clamp((fPosition.y + 1.0) / 10.0, 0.0, 1.0);
  // vec3 hsv = vec3(1.0 - height, 1.0, 1.0);
  // vec3 rgb = hsv2rgb(hsv);

  vec3 color = vec3(0.3, 0.6, 0.9);

  if (fPosition.y > -0.145) {
    vec3 brown = vec3(0.65, 0.35, 0.15);
    vec3 white = vec3(0.9);
    color = mix(brown, white, fPosition.y / 15.0);
  }

  FragColor = vec4(color, 1.0);
  FragNormal = vec4(fNormal / 2 + 0.5, 1.0);
  FragDepth = fDepth;
}
