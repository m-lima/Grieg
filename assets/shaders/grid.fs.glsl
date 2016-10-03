#version 430

in vec3 fPosition;
out vec4 FragColor;

const vec4 gridColor = vec4(1.0, 0.0, 1.0, 0.2);
const vec4 voidColor = vec4(0.0, 0.0, 0.0, 0.0);
const float interval = 50.0;
const float width = 1.0;

bool check(float f)
{
    return int(mod(f / width, interval / width)) == 0;
}

void main()
{
    float line = (check(fPosition.x) || check(fPosition.z)) ? 1.0 : 0.0;

    FragColor = mix(voidColor, gridColor, line);
}
