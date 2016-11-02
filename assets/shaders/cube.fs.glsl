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
    vec3 position;
    vec3 color;
    float intensity;
    vec3 direction;
    float aperture;
};

layout (std140) uniform LightBlock {
    LightSource uLights[12];
};

void main() {
    vec3 texel = vec3(1.0);
    if (uHaveTexture > 0)
        texel = texture(uTexture, fTexCoord).xyz;

    vec3 color = uAmbientLight * texel;
    for (int i = 0; i < 12; i++) {
        vec3 direction;
        float attenuation = 1.0;
        if (uLights[i].type == 0) { // No light
             continue;
        } else if (uLights[i].type == 1) { // Directional light
            direction = normalize(uLights[i].position);
        } else if (uLights[i].type == 2) { // Point light
            direction = normalize(uLights[i].position);
            float dist = distance(uLights[i].position, fPosition);
            attenuation = 1.0 / (1.0 + 0.001 * pow(dist, 2));
        } else if (uLights[i].type == 3) { // Spotlight
            direction = normalize(uLights[i].position);
            float dist = distance(uLights[i].position, fPosition);
            attenuation = 1.0 / (1.0 + 0.0001 * pow(dist, 2));

            float angle = dot(normalize(fPosition - uLights[i].position), normalize(uLights[i].direction));
            if (angle < (1.0 - uLights[i].aperture)) {
              attenuation = 0.0;
            }
            //float angle = degrees(acos(dot(-direction, uLights[i].direction)));
            //if (angle > 1.0) {
            //    attenuation = 0.0;
            //}
        } else {
            continue;
        }

        float diffuseCoeff = dot(fNormal, direction);
        vec3 diffuse = diffuseCoeff * texel * uLights[i].color;

        float specularCoeff = 0.0;
        //if (diffuseCoeff > 0.0) {
        //    vec3 cameraDir = normalize(fEyePos);
        //    vec3 h = normalize(direction + cameraDir);
        //    specularCoeff = pow(max(dot(h, fNormal), 0.0), 128.0);
        //}
        vec3 specular = specularCoeff * uLights[i].color;

        color += uLights[i].intensity * attenuation * (diffuse + specular);
    }

    FragColor = vec4(color, 1.0);
}
