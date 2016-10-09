#version 430

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal; //Already normalized
in vec3 fEyePos;
out vec4 FragColor;

uniform sampler2D uTexture;

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
    vec3 texel = texture(uTexture, fTexCoord).xyz;
    for (int i = 0; i < 12; i++) {
        if (uLights[i].type == 2) { /* Point light */
            vec3 spec = vec3(0.0);
            vec3 n = normalize(fNormal);
            vec3 l = normalize(uLights[i].position - fPosition);
            vec3 e = normalize(fEyePos);
            float intensity = max(dot(l, n), 0.0);

            if (intensity > 0.0) {
                vec3 h = normalize(l + e);
                float intSpec = max(dot(h, n), 0.0);
                spec = vec3(pow(intSpec, 128.0));
            }

            color += max(intensity * uLights[i].color + spec, 0.1) * texel;
        }
    }

    FragColor = vec4(color, 1.0);
}
