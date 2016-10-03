#version 430

in vec3 fPosition;
out vec4 FragColor;

const vec4 gridColor = vec4(1.0, 0.0, 1.0, 0.2);
const vec4 voidColor = vec4(0.0, 0.0, 0.0, 0.0);
const float interval = 50.0;

void main()
{
    // Stolen from http://madebyevan.com/shaders/grid/
    vec2 coord = fPosition.xz / interval;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);

    FragColor = mix(voidColor, gridColor, 1.0 - min(line, 1.0));
}
