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

void main() {
  vec3 texel = vec3(1.0);

  if (uHaveTexture > 0) {
    texel = texture(uTexture, fTexCoord).xyz;
  }

  vec3 normal = fNormal;
  if (uHaveBump > 0) {
    normal = normalize(fNormal + texture(uBump, fTexCoord).xyz);
  }

  // Initialize the color with the ambient lighting
  vec3 color = (uAmbientLight + uAmbient) * texel;

  for (int i = 0; i < 12; i++) {

    // The direction of incidence of light
    vec3 lightIncidence;

    // There is no attenuation to begin with
    float attenuation = 1.0;

    if (uLights[i].type == 0 || uLights[i].type > 3) { // No light
      continue;

    } else if (uLights[i].type == 1) { // Directional light

                                       // There is no point of origin, so the incidence is always the
                                       // reverse of the direction
      lightIncidence = normalize(uLights[i].direction);

    } else { // Light that have a point of origin

             // The direction of incidence is relative to the light source
      lightIncidence = normalize(uLights[i].position - fPosition);

      // Distance to the point of origin
      float dist = distance(uLights[i].position, fPosition);

      if (uLights[i].type == 2) { // Point light

                                  // Point lights attenuate more
        attenuation = 1.0 / (1.0 + 0.0002 * pow(dist, 2));

      } else if (uLights[i].type == 3) { // Spotlight

                                         // Spot lights attenuate less
        attenuation = 1.0 / (1.0 + 0.00005 * pow(dist, 2));

        // Angle between the direction of the light and direction
        // from the light to the surface
        float angle = abs(dot(
          normalize(fPosition - uLights[i].position),
          normalize(uLights[i].direction)));

        // Aplying aperture limit (angle < 1.0 - aperture)
        angle += uLights[i].aperture;

        // Penumbra
        angle = min(angle, 1.0);
        attenuation = pow(angle, 32);

      };
    }

    // Angle between the normal and incidence of light
    float angleOfIncidence = dot(normal, lightIncidence);

    // If the light is facing the normal, illuminate
    if (angleOfIncidence > 0.0) {
      vec3 diffuse = uDiffuse * angleOfIncidence * texel * uLights[i].color;

      // Direction in which the light reflects
      vec3 specularReflection = normalize(
        dot(2 * normal, lightIncidence) * normal - lightIncidence);

      // Angle between the reflection and the viewing angle
      float viewingAngle = dot(
        normalize(fEyePos - fPosition),
        specularReflection);

      // If the light is being reflected towards the eye, calculate
      // the specular color
      vec3 specular = viewingAngle > 0.0 ?
        uSpecular * pow(viewingAngle, uLights[i].specularIndex) * uLights[i].color
        : vec3(0.0);

      color += uLights[i].intensity * attenuation * (diffuse + specular);
    }
  }

  FragColor = vec4(color, 1.0);
  FragNormal = vec4(normal / 2 + 0.5, 1.0);
  //FragDepth = fDepth;
  FragDepth = 0.5;
}
