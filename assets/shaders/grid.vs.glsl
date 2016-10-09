#version 430

layout(location=0) in vec3 vPosition;
out vec3 fPosition;

uniform MatrixBlock {
    mat4 uProj;
    mat4 uView;
};

void main()
{
    fPosition = vPosition;
    gl_Position = uProj * uView * vec4(vPosition, 1.0);
}
