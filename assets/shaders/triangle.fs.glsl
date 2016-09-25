#version 430

uniform vec2 fResolution;
uniform float fGlobalTime;
in vec2 gl_FragCoord;
out vec4 gl_FragColor;

void main() {
    vec2 uv = gl_FragCoord.xy / fResolution.xy;
    gl_FragColor = vec4(uv, 0.5 + 0.5 * sin(fGlobalTime), 1.0);
}
