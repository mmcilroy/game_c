#include "raylib.h"
#include "raymath.h"

#define GLSL_VERSION 330

const int gridSize = 5;
int grid[] = {
    1, 1, 2, 3, 1,
    1, 1, 1, 2, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};

typedef struct Vector2i {
    int x;                // Vector x component
    int y;                // Vector y component
} Vector2i;

void DDA(Vector3 v1, Vector3 v2)
{
    Vector2 rayStart = {v1.x, v1.z};
    Vector2 rayEnd = {v2.x, v2.z};
    Vector2 rayDir = Vector2Normalize(Vector2Subtract(rayEnd, rayStart));
    Vector3 rayDir3 = Vector3Normalize(Vector3Subtract(v2, v1));
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

		Vector2 intersection = Vector2Add(rayStart, Vector2Scale(rayDir, distance));

        float scale = (intersection.x - v1.x) / rayDir3.x;
        float rh = v1.y + (rayDir3.y * scale);
        float gh = grid[mapCheck.y * gridSize + mapCheck.x]-1;

        //if (rh < gh)
        //{
            // move y same distance x
            // seems fine but will only work if angle of light is 45 degrees
            Vector3 p3 = { intersection.x, rh, intersection.y };
            DrawSphere(p3, 0.2, (Color){255, 0, 0, 128});
            //break;
        //}
    }
}

int DrawVoxel(Vector3 vp)
{
    Vector3 p = Vector3Add(vp, (Vector3){0.5, -0.5, 0.5});
    DrawCube(p, 1, 1, 1, BLANK);
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

    Vector3 spherePos = {0.5, 0.5, 0.5};
    //Vector3 lightDir = {1, 1, -1};
    //Vector3 lightPos = Vector3Add(spherePos, Vector3Scale(lightDir, 10));
    Vector3 lightPos = {1.5, 0.5, 2.5};

    InitWindow(screenWidth, screenHeight, "game");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ gridSize/2, 0.0f, gridSize/2 };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    const char* vs = TextFormat("vert.glsl", GLSL_VERSION);
    const char* fs = TextFormat("dda2.glsl", GLSL_VERSION);
    Shader shader = LoadShader(vs, fs);

    int loc = GetShaderLocation(shader, "lightPos");

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        if (IsKeyDown('J')) lightPos.x -= 0.25f;
        if (IsKeyDown('L')) lightPos.x += 0.25f;
        if (IsKeyDown('U')) lightPos.y += 0.25f;
        if (IsKeyDown('O')) lightPos.y -= 0.25f;
        if (IsKeyDown('I')) lightPos.z -= 0.25f;
        if (IsKeyDown('K')) lightPos.z += 0.25f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                BeginShaderMode(shader);
                SetShaderValue(shader, loc, &lightPos, SHADER_UNIFORM_VEC3);

                for (int z=0; z<gridSize; z++)
                {
                    for (int x=0; x<gridSize; x++)
                    {
                        int h = grid[z * gridSize + x];
                        for (int y=0; y<h; y++)
                        {
                            DrawVoxel((Vector3){x, y, z});
                        }
                    }
                }
                EndShaderMode();

                DrawRay((Ray){{0, 0, 0}, {1, 0, 0}}, (Color){ 255, 0, 0, 255 });
                DrawRay((Ray){{0, 0, 0}, {0, 1, 0}}, (Color){ 0, 255, 0, 255 });
                DrawRay((Ray){{0, 0, 0}, {0, 0, 1}}, (Color){ 0, 0, 255, 255 });
                DrawSphere((Vector3){ 0, 0, 0 }, 0.2, BLACK);

                DrawSphere(spherePos, 0.2, GREEN);
                DrawSphere(lightPos, 0.2, YELLOW);
                //DrawRay((Ray){spherePos, lightDir}, BLACK);
                DrawRay((Ray){spherePos, Vector3Normalize(Vector3Subtract(lightPos, spherePos))}, BLACK);

                DDA(spherePos, lightPos);

            EndMode3D();

            DrawFPS(10, 10);
            DrawText(TextFormat("(%.02f, %.02f, %.02f) -> (%.02f, %.02f, %.02f)", spherePos.x, spherePos.y, spherePos.z, lightPos.x, lightPos.y, lightPos.z), 20, 40, 20, BLACK);
            //DrawText(TextFormat("(%.02f, %.02f)", i.x, i.y), 20, 80, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
