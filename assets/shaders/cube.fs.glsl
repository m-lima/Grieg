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
    vec3 normal;
    vec3 color;
    float intensity;
    vec3 _pad1;
};

layout (std140) uniform LightBlock {
    LightSource uLights[12];
};

void main() {
    vec3 color = vec3(0.0);
    vec3 texel = vec3(1.0);
    if (uHaveTexture > 0)
        texel = texture(uTexture, fTexCoord).xyz;

    float accum = 0.0;
    for (int i = 0; i < 12; i++) {
        if (uLights[i].type == 2) { /* Point light */
            vec3 n = normalize(fNormal);
            vec3 l = normalize(uLights[i].position - fPosition);
            vec3 e = normalize(fEyePos);
            float dist = distance(uLights[i].position, fPosition);
            float att = 1.0 / (1.0 - 0.00001 * dist * dist);

            float intensity = max(dot(n, l), 0.0);
            if (intensity > 0.0) {
                vec3 h = normalize(l + e);
                float specularIntensity = max(dot(h, n), 0.0);
                intensity += 0.5 * pow(specularIntensity, 128.0);
            }

            accum += max(intensity * att, 0.0);
        }
    }
    FragColor = vec4(texel * accum, 1.0);
}
