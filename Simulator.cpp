
#include "Fluid.h"

static bool use_edges = true;
bool use_colour_shader = true;
const char* WIN_NAME = "Fluid Sim";
FluidGrid grid;

void keybinds(){
    //R resets the simulation
    if (IsKeyPressed(KEY_R)) {
        grid = FluidGrid();
        grid.init(true, "colours.fs");
    }
    //E toggles edge detection
    if (IsKeyPressed(KEY_E)){
        use_edges = !use_edges;
        grid = FluidGrid();
        grid.init(true, "colours.fs");
    }
    //S toggles shaders
    if (IsKeyPressed(KEY_S)) use_colour_shader = !use_colour_shader;
    grid.apply_mouse();
}

int main() {
    InitWindow(WIDTH, HEIGHT, WIN_NAME);
    ToggleFullscreen();
    SetTargetFPS(FPS);
    srand(clock());
    grid.init(true, "colours.fs");

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        keybinds();

        grid.update();
        //grid.apply_random_force();

        DrawFPS(30, 30);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}