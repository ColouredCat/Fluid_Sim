
#include "Fluid.h"

const char* WIN_NAME = "Fluid Sim";
//macro to easily set a float uniform from its name
int set_uniform(const char* name ,float value, Shader s){
    int loc = GetShaderLocation(s, name);
    if (loc == -1) { 
        printf("Could not find uniform \"%s\"\n", name);
        return -1;
    }
    const float valc[1] = {value};
    SetShaderValue(s, loc, valc, SHADER_ATTRIB_FLOAT);
    return loc;
}

bool use_edges = true;
FluidGrid grid(use_edges);
Shader s;

void toggle_edge(){
    if (IsKeyPressed(KEY_E)){
        use_edges = !use_edges;
        grid = FluidGrid(use_edges);
    }
}

void reset(){
    if (IsKeyPressed(KEY_R)) grid = FluidGrid(use_edges);
}

int main() {
    InitWindow(WIDTH, HEIGHT, WIN_NAME);
    ToggleFullscreen();
    SetTargetFPS(FPS);
    srand(clock());
    Vector2 mpos;

    s = LoadShader(NULL, "colours.fs");
    if (!IsShaderValid(s)) { return 1; }

    set_uniform("u_width", WIDTH, s);
    set_uniform("u_height", HEIGHT, s);

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        //BeginShaderMode(s);;
        //grid.draw_grid(true, true, true);
        grid.update();
        toggle_edge();
        reset();
        //grid.apply_random_force();
        grid.apply_mouse();
        //grid.apple_edge_force();

        //EndShaderMode();
        DrawFPS(30, 30);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}