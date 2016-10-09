#version 430

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal; //Already normalized
in vec3 fEyePos;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform int uHaveTexture;

struct LightSource {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float ambientCoeff;
    vec3 _pad1;
};

layout (std140) uniform LightBlock {
    LightSource uLights[12];
};

void main() {
    vec3 texel = vec3(1.0);
    if (uHaveTexture > 0)
        texel = texture(uTexture, fTexCoord).xyz;

    vec3 color = vec3(0.0);
    for (int i = 0; i < 12; i++) {
        vec3 direction;
        float attenuation = 1.0;
        if (uLights[i].type == 0) { /* No light */
             continue;
        } else if (uLights[i].type == 1) { /* Directional light */
            direction = normalize(uLights[i].position);
        } else if (uLights[i].type == 2) { /* Point light */
            direction = normalize(uLights[i].position - fPosition);
            float dist = distance(uLights[i].position, fPosition);
            attenuation = 1.0 / (1.0 + 0.0001 * pow(dist, 2));
        } else if (uLights[i].type == 3) { /* Spotlight */
            direction = normalize(uLights[i].position - fPosition);
            float dist = distance(uLights[i].position, fPosition);
            attenuation = 1.0 / (1.0 + 0.0001 * pow(dist, 2));

            float angle = degrees(acos(dot(-direction, uLights[i].direction)));
            if (angle > 1.0) {
                attenuation = 0.0;
            }
        } else {
            continue;
        }

        vec3 ambient = uLights[i].ambientCoeff * texel.rgb * uLights[i].color;

        float diffuseCoeff = max(dot(fNormal, direction), 0.0);
        vec3 diffuse = diffuseCoeff * texel.rgb * uLights[i].color;

        float specularCoeff = 0.0;
        if (diffuseCoeff > 0.0) {
            vec3 cameraDir = normalize(fEyePos);
            vec3 h = normalize(direction + cameraDir);
            specularCoeff = pow(max(dot(h, fNormal), 0.0), 128.0);
        }
        vec3 specular = specularCoeff * uLights[i].color;

        color += ambient + attenuation * (diffuse + specular);
    }

    FragColor = vec4(color, 1.0);
}
