#version 430

in vec2 fTexCoord;

out vec4 FragColor;

uniform sampler2D uFont;

void main()
{
    FragColor = texture(uFont, fTexCoord);
}
