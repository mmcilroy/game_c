#include "raylib.h"
#include "raymath.h"

#define GLSL_VERSION 330

const int worldSize = 10;
const int worldSizeSquared = worldSize * worldSize;
const int worldSizeCubed = worldSize * worldSize * worldSize;

typedef struct Vector2i {
    int x;                // Vector x component
    int y;                // Vector y component
} Vector2i;

typedef struct Vector3i {
    int x;                // Vector x component
    int y;                // Vector y component
    int z;                // Vector y component
} Vector3i;

Vector3 intersections[30];
Vector3i intersections2[30];

int WorldIndex(Vector3i p)
{
    return p.z * worldSizeSquared +
           p.y * worldSize +
           p.x;
}

void SetWorld(Vector3i p, int v, int* world)
{
    int i = WorldIndex(p);
    if (i >= 0 && i < worldSizeCubed) {
        world[i] = v;
    }
}

void DrawVoxel(Vector3i p, int* world)
{
    Vector3 cp = Vector3Add((Vector3){p.x, p.y, p.z}, (Vector3){0.5, 0.5, 0.5});

    int i = WorldIndex(p);

    if (i >= 0 && i < worldSizeCubed && world[i])
    {
        DrawCube(cp, 1, 1, 1, (Color){255, 0, 0, 64});
    }
    else
    {
        //DrawCubeWires(cp, 1, 1, 1, GRAY);
    }
}

void DDAX(Vector3 start, Vector3 end, int* world)
{
    Vector3 dir = Vector3Normalize(Vector3Subtract(end, start));
    Vector3 pos = start;

    int i = 0;
    while(pos.x < 9 && pos.y < 9 && pos.z < 9) {
        // how far do we have to travel along dir until we get to next xyz grid position
        float distX = ((int)pos.x + 1 - pos.x) / dir.x;
        float distY = ((int)pos.y + 1 - pos.y) / dir.y;
        float distZ = ((int)pos.z + 1 - pos.z) / dir.z;

        // travel along the shortest distance
        if(distX <= distY && distX <= distZ) {
            pos = Vector3Add(pos, Vector3Scale(dir, distX));
        } else if(distY <= distX && distY <= distZ) {
            pos = Vector3Add(pos, Vector3Scale(dir, distY));
        } else {
            pos = Vector3Add(pos, Vector3Scale(dir, distZ));
        }

        Vector3i worldPos = {(int)pos.x, (int)pos.y, (int)pos.z};
        SetWorld(worldPos, 1, world);
        DrawSphere(pos, 0.2, (Color){255, 0, 0, 128});
        intersections2[i] = worldPos;
        intersections[i++] = pos;
    }
}

void DDA2D(Vector3 v1, Vector3 v2, int* world)
{
    Vector2 rayStart = {v1.x, v1.z};
    Vector2 rayEnd = {v2.x, v2.z};
    Vector2 rayDir = Vector2Normalize(Vector2Subtract(rayEnd, rayStart));
    Vector2 rayUnitStepSize = { 
        sqrt(1 + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)), 
        sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y))
    };

    Vector2i mapCheck = {(int)rayStart.x, (int)rayStart.y};
    Vector2 rayLength1D;
    Vector2i step;

    if (rayDir.x < 0)
    {
        step.x = -1;
        rayLength1D.x = (rayStart.x - (float)mapCheck.x) * rayUnitStepSize.x;
    }
    else
    {
        step.x = 1;
        rayLength1D.x = ((float)mapCheck.x + 1.0f - rayStart.x) * rayUnitStepSize.x;
    }

    if (rayDir.y < 0)
    {
        step.y = -1;
        rayLength1D.y = (rayStart.y - (float)mapCheck.y) * rayUnitStepSize.y;
    }
    else
    {
        step.y = 1;
        rayLength1D.y = ((float)mapCheck.y + 1.0f - rayStart.y) * rayUnitStepSize.y;
    }

    // Perform "Walk" until collision or range check
    float maxDistance = 10.0f;
    float distance = 0.0f;
    int i = 0;
    while (distance < maxDistance-1)
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

		Vector2 intersection = Vector2Add(rayStart, Vector2Scale(rayDir, distance));
        Vector3 p3 = { intersection.x, 0.5, intersection.y };
        intersections[i++] = p3;
        DrawSphere(p3, 0.2, (Color){255, 0, 0, 128});
    }
}

void DDA3D(Vector3 from, Vector3 to, int* world)
{
    Vector3 rayDir = Vector3Normalize(Vector3Subtract(to, from));

    // distance the ray has to travel to go from 1 xyz side to the next
    Vector3 rayUnitStepSize = {
        sqrt(1 + (rayDir.z * rayDir.z) / (rayDir.x * rayDir.x)), 
        sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y)),
        sqrt(1 + (rayDir.y / rayDir.y) / (rayDir.z / rayDir.z))
    };

    Vector3i mapCheck = { (int)from.x, (int)from.y, (int)from.z };
    Vector3i step;
    Vector3 rayLength1D = {0, 0, 0};

    if (rayDir.x < 0)
    {
        step.x = -1;
        rayLength1D.x = (from.x - (float)mapCheck.x) * rayUnitStepSize.x;
    }
    else
    {
        step.x = 1;
        rayLength1D.x = ((float)mapCheck.x + 1.0f - from.x) * rayUnitStepSize.x;
    }

    if (rayDir.y < 0)
    {
        step.y = -1;
        rayLength1D.y = (from.y - (float)mapCheck.y) * rayUnitStepSize.y;
    }
    else
    {
        step.y = 1;
        rayLength1D.y = ((float)mapCheck.y + 1.0f - from.y) * rayUnitStepSize.y;
    }

    if (rayDir.z < 0)
    {
        step.z = -1;
        rayLength1D.z = (from.z - (float)mapCheck.z) * rayUnitStepSize.z;
    }
    else
    {
        step.z = 1;
        rayLength1D.z = ((float)mapCheck.z + 1.0f - from.z) * rayUnitStepSize.z;
    }

    // Perform "Walk" until collision or range check
    float maxDistance = worldSize - 1.5;
    float distance = 0.0f;
    //while (distance < maxDistance)

    while (true)
    {
        bool walkedX = false;
        bool walkedY = false;
        bool walkedZ = false;

        // Walk along shortest path
        if (rayLength1D.x < rayLength1D.y && rayLength1D.x < rayLength1D.z)
        {
            mapCheck.x += step.x;
            distance = rayLength1D.x;
            rayLength1D.x += rayUnitStepSize.x;
            walkedX = true;
        }
        else
        if (rayLength1D.y < rayLength1D.x && rayLength1D.y < rayLength1D.z)
        {
            mapCheck.y += step.y;
            distance = rayLength1D.y;
            rayLength1D.y += rayUnitStepSize.y;
            walkedY = true;
        }
        else
        //if (rayLength1D.z < rayLength1D.x && rayLength1D.z < rayLength1D.y)
        {
            mapCheck.z += step.z;
            distance = rayLength1D.z;
            rayLength1D.z += rayUnitStepSize.z;
            walkedZ = true;
        }

		Vector3 intersection = Vector3Add(from, Vector3Scale(rayDir, distance));
        //SetWorld((Vector3i){(int)intersection.x, (int)intersection.y, (int)intersection.z}, 1, world);
        //SetWorld(mapCheck, 1, world);
        DrawSphere(intersection, 0.2, (Color){255, 255, 0, 255});

        if (mapCheck.x > worldSize ||
            mapCheck.y > worldSize ||
            mapCheck.z > worldSize) {
            step.x = 0;
            break;
        }
    }

    step.x = 0;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1600;
    const int screenHeight = 900;

    Vector3 startPos = {0.5, 0.5, 0.5};
    Vector3 endPos = {7.5, 5.5, 5.5}; //{worldSize - 0.5, worldSize - 0.5, worldSize - 0.5};

    int world[worldSizeCubed];
    //world[0] = 1;
    //world[worldSizeSquared] = 1;
    //world[worldSizeCubed-1] = 1;

    InitWindow(screenWidth, screenHeight, "game");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ worldSize/2, 0.0f, worldSize/2 };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        for(int i = 0; i < worldSizeCubed; i++) {
            world[i] = 0;
        }

        for(int i=0; i<30; i++) {
            intersections[i] = (Vector3){0, 0, 0};
            intersections2[i] = (Vector3i){0, 0, 0};
        }

        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        if (IsKeyPressed('J')) endPos.x -= 1;
        if (IsKeyPressed('L')) endPos.x += 1;
        if (IsKeyPressed('U')) endPos.y += 1;
        if (IsKeyPressed('O')) endPos.y -= 1;
        if (IsKeyPressed('I')) endPos.z -= 1;
        if (IsKeyPressed('K')) endPos.z += 1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DDAX(startPos, endPos, world);
                //DDA2D(startPos, endPos, world);

                for (int z=0; z<worldSize; z++)
                {
                    for (int y=0; y<worldSize; y++)
                    {
                        for (int x=0; x<worldSize; x++)
                        {
                            DrawVoxel((Vector3i){x, y, z}, world);
                        }
                    }
                }

                DrawRay((Ray){{0, 0, 0}, {1, 0, 0}}, (Color){ 255, 0, 0, 255 });
                DrawRay((Ray){{0, 0, 0}, {0, 1, 0}}, (Color){ 0, 255, 0, 255 });
                DrawRay((Ray){{0, 0, 0}, {0, 0, 1}}, (Color){ 0, 0, 255, 255 });
                DrawSphere((Vector3){ 0, 0, 0 }, 0.2, BLACK);

                DrawSphere(startPos, 0.25, GREEN);
                DrawSphere(endPos, 0.25, BLUE);
                DrawRay((Ray){startPos, Vector3Subtract(endPos, startPos)}, BLACK);

            EndMode3D();

            DrawFPS(10, 10);
            DrawText(TextFormat("(%.02f, %.02f, %.02f) -> (%.02f, %.02f, %.02f)", startPos.x, startPos.y, startPos.z, endPos.x, endPos.y, endPos.z), 20, 40, 20, BLACK);

            for(int i=0; i<20; i++)
            {
                DrawText(TextFormat("(%.04f, %.04f, %.04f)", intersections[i].x, intersections[i].y, intersections[i].z), 20, 100+i*20, 20, BLACK);
                DrawText(TextFormat("(%d, %d, %d)", intersections2[i].x, intersections2[i].y, intersections2[i].z), 320, 100+i*20, 20, BLACK);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
