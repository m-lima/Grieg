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

void main()
{
  vec3 normal = fNormal;
  if (uHaveBump > 0) {
    normal = normalize(fNormal + texture(uBump, fTexCoord).xyz);
  }

  FragColor = vec4(normal, 1.0);
  FragNormal = vec4(normal / 2 + 0.5, 1.0);
  FragDepth = fDepth;
}
