#include "raylib.h"
#include "raymath.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define GLSL_VERSION 330

bool restart = true;
bool debug = false;

void DumpLuaStack(lua_State *L)
{
    printf("STACK\n");
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++)
    {
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i))
        {
        case LUA_TNUMBER:
            printf("%g\n", lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            printf("%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        default:
            printf("%p\n", lua_topointer(L, i));
            break;
        }
    }
}

void CalcMeshNormals(Mesh *mesh)
{
    float *mv = mesh->vertices;
    float *mn = mesh->normals;
    for (int i = 0; i < mesh->vertexCount * 3; i += 9)
    {
        Vector3 v0 = {mv[i + 0], mv[i + 1], mv[i + 2]};
        Vector3 v1 = {mv[i + 3], mv[i + 4], mv[i + 5]};
        Vector3 v2 = {mv[i + 6], mv[i + 7], mv[i + 8]};
        Vector3 tangentA = Vector3Subtract(v1, v0);
        Vector3 tangentB = Vector3Subtract(v2, v0);
        Vector3 normal = Vector3Normalize(Vector3CrossProduct(tangentA, tangentB));

        mn[i + 0] = normal.x;
        mn[i + 1] = normal.y;
        mn[i + 2] = normal.z;
        mn[i + 3] = normal.x;
        mn[i + 4] = normal.y;
        mn[i + 5] = normal.z;
        mn[i + 6] = normal.x;
        mn[i + 7] = normal.y;
        mn[i + 8] = normal.z;

        for (int j = 0; j < 9; j++)
        {
            //printf("normal %d: %.02f\n", i + j, mn[i + j]);
        }
    }
}

void LoadLuaMesh(Mesh *mesh, const char *filename)
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    int err = luaL_dofile(L, filename);
    if (err)
    {
        printf("LoadLuaMesh error: %s\n", luaL_checkstring(L, -1));
        return;
    }

    lua_getglobal(L, "vertices");
    lua_len(L, -1);
    int len = lua_tonumber(L, -1);
    lua_pop(L, 1);

    mesh->vertices = (float *)RL_MALLOC(len * sizeof(float));
    mesh->normals = (float *)RL_MALLOC(len * sizeof(float));
    mesh->vertexCount = len / 3;
    mesh->triangleCount = mesh->vertexCount / 3;

    printf("vertexCount: %d\n", mesh->vertexCount);
    printf("triangleCount: %d\n", mesh->triangleCount);

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        float n = lua_tonumber(L, -1);
        mesh->vertices[i++] = n;
        lua_pop(L, 1);
        //printf("vertex %d: %.02f\n", i - 1, n);
    }

    lua_getglobal(L, "colors");
    lua_len(L, -1);
    len = lua_tonumber(L, -1);
    lua_pop(L, 1);

    mesh->colors = (unsigned char *)RL_MALLOC(len * sizeof(unsigned char));

    i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        unsigned char n = lua_tonumber(L, -1);
        mesh->colors[i++] = n;
        //printf("color %d: %d\n", i - 1, n);
        lua_pop(L, 1);
    }

    CalcMeshNormals(mesh);
    UploadMesh(mesh, false);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    const int gridWidth = 3;
    const int gridHeight = 3;

    /*
    int heights[9] = {
        1, 2, 3,
        2, 3, 4,
        3, 4, 5
    };
    */

    int heights[9] = {
        1, 1, 1,
        1, 3, 1,
        1, 1, 1
    };

    while (restart)
    {
        InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");
        DisableCursor();

        restart = false;

        // Define the camera to look into our 3d world
        Camera3D camera = {0};
        camera.position = (Vector3){0.0f, 10.0f, 10.0f}; // Camera position
        camera.target = (Vector3){0.0f, 0.0f, 0.0f};     // Camera looking at point
        camera.up = (Vector3){0.0f, 1.0f, 0.0f};         // Camera up vector (rotation towards target)
        camera.fovy = 60.0f;                             // Camera field-of-view Y
        camera.projection = CAMERA_PERSPECTIVE;          // Camera mode type

        SetTargetFPS(60); // Set our game to run at 60 frames-per-second

        const char* vs = TextFormat("vert.glsl", GLSL_VERSION);
        const char* fs = TextFormat("game.glsl", GLSL_VERSION);
        Shader shader = LoadShader(vs, fs);

        for (int i = 0; i < 9; i++)
        {
            int h = heights[i];
            int loc = GetShaderLocation(shader, TextFormat("heights[%i]", i));
            SetShaderValue(shader, loc, &h, SHADER_UNIFORM_INT);
            printf("loc %d %d = %d\n", i, loc, h);
        }

        Mesh mesh = {0};
        LoadLuaMesh(&mesh, "mesh/cube.lua");
        printf("vertexCount: %d\n", mesh.vertexCount);
        printf("triangleCount: %d\n", mesh.triangleCount);

        Model model = LoadModelFromMesh(mesh);
        model.materials[0].shader = shader;

        // Main game loop
        while (!WindowShouldClose()) // Detect window close button or ESC or R key
        {
            // Update
            //----------------------------------------------------------------------------------
            UpdateCamera(&camera, CAMERA_THIRD_PERSON); // Update camera

            if (IsKeyPressed(KEY_R))
            {
                restart = true;
                break;
            }

            if (IsKeyPressed(KEY_SLASH))
            {
                debug = !debug;
            }

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(SKYBLUE);

            BeginMode3D(camera);

            BeginShaderMode(shader);

            for (int x = 0; x < gridWidth; x++)
            {
                for (int z = 0 ; z < gridHeight; z++)
                {
                    //DrawModel(model, (Vector3){x, heights[z*3+x]-1, -z-1}, 1, BLANK);

                    for (int y = 0; y < heights[z*3+x]; y++)
                    {
                        DrawModel(model, (Vector3){x, y, -z-1}, 1, BLANK);
                    }
                }
            }

            EndShaderMode();

            if (debug)
            {
                float *mv = mesh.vertices;
                float *mn = mesh.normals;
                for (int i = 0; i < mesh.vertexCount * 3; i += 3)
                {
                    Vector3 p = {mv[i + 0], mv[i + 1], mv[i + 2]};
                    Vector3 d = {mn[i + 0], mn[i + 1], mn[i + 2]};
                    DrawRay((Ray){p, d}, PURPLE);
                }
            }

            DrawGrid(10, 1.0f);
            DrawRay((Ray){ {5, 0, 0}, {0, 1, 0} }, RED);
            DrawRay((Ray){ {0, 0, -5}, {0, 1, 0} }, BLUE);
            DrawRay((Ray){ {0.5, 1, -0.5}, {1, 1, 1} }, BLACK);

            EndMode3D();

            DrawFPS(10, 10);

            EndDrawing();
            //----------------------------------------------------------------------------------
        }

        // De-Initialization
        //--------------------------------------------------------------------------------------
        UnloadModel(model);

        CloseWindow(); // Close window and OpenGL context
    }

    return 0;
}
