#version 430

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal; //Already normalized
in vec3 fEyePos;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform int uHaveTexture;

uniform vec3 uAmbientLight;

struct LightSource {
  int type;
  float specularIndex;
  float specularLevel;
  float diffuseLevel;
  vec3 direction;
  float aperture;
  vec3 color;
  float intensity;
  vec3 position;
};

layout(std140) uniform LightBlock{
    LightSource uLights[12];
};

void main() {
  vec3 texel = vec3(1.0);

  if (uHaveTexture > 0) {
    texel = texture(uTexture, fTexCoord).xyz;
  }

  // Initialize the color with the ambient lighting
  vec3 color = uAmbientLight * texel;

  for (int i = 0; i < 12; i++) {

    // The direction of incidence of light
    vec3 lightIncidence;

    // There is no attenuation to begin with
    float attenuation = 1.0;

    if (uLights[i].type == 0 || uLights[i].type > 3) { // No light
      continue;

    } else if (uLights[i].type == 1) { // Directional light

      // There is no point of origin, so the incidence is always the reverse of the direction
      lightIncidence = normalize(uLights[i].direction);

    } else { // Light that have a point of origin

      // The direction of incidence is relative to the light source
      lightIncidence = normalize(uLights[i].position - fPosition);

      // Distance to the point of origin
      float dist = distance(uLights[i].position, fPosition);

      if (uLights[i].type == 2) { // Point light

        // Point lights attenuate more
        attenuation = 1.0 / (1.0 + 0.0005 * pow(dist, 2));

      } else if (uLights[i].type == 3) { // Spotlight

        // Spot lights attenuate less
        attenuation = 1.0 / (1.0 + 0.00005 * pow(dist, 2));

        // Angle between the direction of the light and direction from the light to the surface
        float angle = dot(normalize(fPosition - uLights[i].position), normalize(uLights[i].direction));

		// Aplying aperture limit (angle < 1.0 - aperture)
		angle += uLights[i].aperture;

        // Penumbra
		angle = min(angle, 1.0);
        attenuation = pow(angle, 16);

      };
    }

    // Angle between the normal and incidence of light
    float angleOfIncidence = dot(fNormal, lightIncidence);

    // If the light is facing the normal, illuminate
    if (angleOfIncidence > 0.0) {
      vec3 diffuse = uLights[i].diffuseLevel * angleOfIncidence * texel * uLights[i].color;

      // Direction in which the light reflects
      vec3 specularReflection = normalize(dot(2 * fNormal, lightIncidence) * fNormal - lightIncidence);

      // Angle between the reflection and the viewing angle
      float viewingAngle = dot(normalize(fEyePos - fPosition), specularReflection);

      // If the light is being reflected towards the eye, calculate the specular color
      vec3 specular = viewingAngle > 0.0 ? uLights[i].specularLevel * pow(viewingAngle, uLights[i].specularIndex) * uLights[i].color : vec3(0.0);

      color += uLights[i].intensity * attenuation * (diffuse + specular);
    }
  }

  FragColor = vec4(color, 1.0);
}
