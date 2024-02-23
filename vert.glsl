#version 330

layout (location=0) in vec3 position;
layout (location=2) in vec3 normal;
layout (location=3) in vec3 color;

out vec3 outColor;
out vec4 modelPosition;
out vec4 worldPosition;

uniform mat4 mvp;
uniform mat4 matModel;

void main()
{
    outColor = color;
    modelPosition = vec4(position, 1.0);
    worldPosition = matModel * modelPosition;
    gl_Position = mvp * modelPosition;
}
