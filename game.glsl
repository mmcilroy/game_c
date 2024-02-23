#version 330

in vec3 outColor;
in vec4 modelPosition;
in vec4 worldPosition;

out vec4 fragColor;

uniform int heights[9];

void main()
{
    int x = int(worldPosition.x);
    int z = int(worldPosition.z);

    if (x == 0) {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (x == 1) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (x == 2) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
