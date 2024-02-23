#version 330

in vec3 outColor;
in vec4 modelPosition;
in vec4 worldPosition;

out vec4 fragColor;

const vec3 lightDir = vec3(1, 1, -1);

const int gridSize = 5;
int grid[] = {
    1, 1, 2, 3, 1,
    1, 1, 1, 2, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};

uniform vec3 lightPos;

bool inShadow(vec3 v1)
{
    vec3 v2 = lightPos; //v1.xyz + lightDir * 10;
    vec2 rayStart = v1.xz;
    vec2 rayEnd = v2.xz;
    vec2 rayDir = normalize(rayEnd - rayStart);
    vec3 rayDir3 = normalize(v2 - v1);
    vec2 rayUnitStepSize = { sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)), sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)) };
    vec2 rayLength1D;

    ivec2 mapCheck = ivec2(int(rayStart.x), int(rayStart.y));
    ivec2 step = ivec2(0, 0);

    if (rayDir.x < 0)
    {
        step.x = -1;
        rayLength1D.x = (rayStart.x - float(mapCheck.x)) * rayUnitStepSize.x;
    }
    else
    {
        step.x = 1;
        rayLength1D.x = (float(mapCheck.x) + 1.0f - rayStart.x) * rayUnitStepSize.x;
    }

    if (rayDir.y < 0)
    {
        step.y = -1;
        rayLength1D.y = (rayStart.y - float(mapCheck.y)) * rayUnitStepSize.y;
    }
    else
    {
        step.y = 1;
        rayLength1D.y = (float(mapCheck.y) + 1.0f - rayStart.y) * rayUnitStepSize.y;
    }

    // Perform "Walk" until collision or range check
    float maxDistance = 10.0f;
    float distance = 0.0f;
    while (distance < maxDistance)
    {
        // Walk along shortest path
        if (rayLength1D.x < rayLength1D.y)
        {
            mapCheck.x += step.x;
            distance = rayLength1D.x;
            rayLength1D.x += rayUnitStepSize.x;
        }
        else
        {
            mapCheck.y += step.y;
            distance = rayLength1D.y;
            rayLength1D.y += rayUnitStepSize.y;
        }

        vec2 intersection = rayStart + rayDir * distance;

        int i = (gridSize * int(mapCheck.y)) + int(mapCheck.x);
        int gh = 0;
        if (i >= 0 && i < (gridSize * gridSize)) {
            gh = grid[i]-1;
        }

        float scale = (intersection.x - v1.x) / rayDir3.x;
        float rh = v1.y + (rayDir3.y * scale);

        if (rh < float(gh))
        {
            return true;
        }
    }

    return false;
}

vec4 colorGradient(vec3 p) {
    return vec4(vec3(p.x, p.y, p.z)/gridSize, 1.0);
}

vec4 colorFlat(vec3 p) {
    return vec4(vec3(int(p.x), int(p.y), int(p.z))/gridSize, 1.0);
}

vec4 colorGridHeight(vec3 p) {
    int i = (gridSize * int(p.z)) + int(p.x);
    int h = 0;
    if (i >= 0 && i < (gridSize * gridSize)) {
        h = grid[i];
    }
    return vec4(vec3(h, 0, 0)/gridSize, 1.0);
}

void main()
{
    //fragColor = colorFlat(modelPosition.xyz);

    if (modelPosition.y < 0) {
        fragColor = vec4(0, 0, 0, 1);
    } else if (inShadow(modelPosition.xyz)) {
        fragColor = vec4(0, 0.25, 0, 1);
    } else {
        fragColor = vec4(0, 1, 0, 1);
    }
}
