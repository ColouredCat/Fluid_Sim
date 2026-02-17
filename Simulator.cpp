
#include "Fluid.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    const char* FRAGMENT_SHADER = "resorces/colours_web.fs";
    const char* VERTEX_SHADER = "resorces/default_web.vs";
    #define GLSL_VERSION 300
#else
    const char* FRAGMENT_SHADER = "resorces/colours.fs";
    const char* VERTEX_SHADER = "resorces/default.vs";
    #define GLSL_VERSION 330
#endif

static bool use_edges = true;
bool use_colour_shader = true;
const char* WIN_NAME = "Fluid Sim";
FluidGrid grid;

void keybinds(){
    //R resets the simulation
    if (IsKeyPressed(KEY_R)) {
        grid.reset();
    }
    //E toggles edge detection
    if (IsKeyPressed(KEY_E)){
        use_edges = !use_edges;
        grid.reset();
    }
    //S toggles shaders
    if (IsKeyPressed(KEY_S)) use_colour_shader = !use_colour_shader;
    grid.apply_mouse();
}

void mainloop(){
    BeginDrawing();
    ClearBackground(BLACK);
    keybinds();

    grid.update();
    //grid.apply_random_force();

    //DrawFPS(30, 30);
    EndDrawing();
}

int main() {
    InitWindow(WIDTH, HEIGHT, WIN_NAME);
    srand(clock());
    grid.init(use_edges, FRAGMENT_SHADER, VERTEX_SHADER);

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(mainloop, 0, 1);
    #else
        ToggleFullscreen();
        SetTargetFPS(FPS);
        while (!WindowShouldClose()){
            mainloop();
        }
    #endif

    CloseWindow();
    return 0;
}