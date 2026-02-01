
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <stdbool.h>
#include <time.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;
const char* WIN_NAME = "Fluid Sim";
const int FPS = 60;
const float TIMESTEP = (1 / (float)FPS);

const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 20;
const int GRID_SPACING_X = (WIDTH / GRID_WIDTH);
const int GRID_SPACING_Y = (HEIGHT / GRID_HEIGHT);

const float OVERRELAX = 1.8f;
const int ITERATIONS = 30;
const float GRAVITY = 9.81f;

const int POINT_SIZE = 3;
const Color LINE_COL = GRAY;
const Color CELL_COL = BLUE;
const Color ARROW_COL = RED;
const Color PARTICLE_COL = GREEN;

const float INITIAL_VEL = (rand() % 10) - 5;
const double VEL_SCALE = 1;

class GridPoint {
    public:   
    Vector2 grid_pos;
    Vector2 screen_pos;
    Vector2 particle_pos;
    bool is_v_vertical;

    float v = 0;
    float d = 0;
    float s = 1;

    void init(Vector2 position, bool edge_boundries) {
        //calculate the cell's position on screen from it's array position
        grid_pos = position;
        screen_pos.x = (GRID_SPACING_X * position.x);
        screen_pos.y = (GRID_SPACING_Y * position.y);
        v = INITIAL_VEL;
        //cells on every second row show vertical velocity
        is_v_vertical = ((int)position.y % 2);
        if (is_v_vertical) screen_pos.x += GRID_SPACING_X/2;
        //mark cells on the outside as bariers with no velocity
        if ((grid_pos.x == 0 || grid_pos.x == GRID_WIDTH-1 || grid_pos.y == 0 || grid_pos.y == GRID_HEIGHT-1) && edge_boundries){
            s = 0;
            v = 0;
        }
    }

    void draw_grid(bool draw_grid_lines, bool draw_arrows, bool draw_text){
        //only draw divergence for cells not on the edge
        if (s != 0 && draw_text){
            char str[20];
            sprintf(str, "%f", v);
            DrawText(str, screen_pos.x + 10, screen_pos.y + 10, 10, GRAY);
        }
        //draw horizontal and vertical grid lines only on the outer rows
        if(draw_grid_lines && grid_pos.x == 0 && is_v_vertical) DrawLine(0, screen_pos.y, WIDTH, screen_pos.y, LINE_COL);
        if(draw_grid_lines && grid_pos.y == 0) DrawLine(screen_pos.x, 0, screen_pos.x, HEIGHT, LINE_COL);

        if (draw_arrows){
            DrawCircleV(screen_pos, POINT_SIZE, CELL_COL);
            if (is_v_vertical) DrawLineV(screen_pos, (Vector2){screen_pos.x, screen_pos.y + v}, ARROW_COL);
            if (!is_v_vertical) DrawLineV(screen_pos, (Vector2){screen_pos.x + v, screen_pos.y}, ARROW_COL);
        }
    }
};

class FluidGrid {
    private:

    void force_incompressable() {
        float d, s;
        //run the calcualations multiple times as the new velocity values will change the divergence
        for (int i = 0; i < ITERATIONS; i++) {
            //update all cells, ignoring the outermost ones
            for (int i = 0; i < GRID_WIDTH-1; i+=1) {
                for (int j = 1; j < GRID_HEIGHT-1; j+=1){
                    /*step 1 : force incompressability by calculating the divergence, 
                    i.e how much fluid is flowing in an out of the cell*/
                    d = OVERRELAX * (cells[i+1][j].v - cells[i][j].v + cells[i][j+1].v - cells[i][j-1].v);
                    cells[i][j].d = d;
                    s = cells[i+1][j].s + cells[i][j].s + cells[i][j+1].s + cells[i][j-1].s;
                    //update velocity values based on this
                    cells[i][j].v += d*cells[i][j].s/s;
                    cells[i][j+1].v -= d*cells[i][j+1].s/s;
                    cells[i+1][j].v -= d*cells[i+1][j].s/s;
                    cells[i][j-1].v += d*cells[i][j-1].s/s;
                }
            }
        }
    }

    void advect_particles() {
        Vector2 v;
        Vector2 pos;
        float u;
        float w00, w01, w10, w11;
        for (int i = 0; i < GRID_WIDTH-1; i++){
            for (int j = 1; j < GRID_HEIGHT-1; j+=1) {

                if (cells[i-1][j+1].is_v_vertical){
                    v.x = (cells[i-1][j+1].v + cells[i-1][j-1].v + cells[i+1][j+1].v + cells[i+1][j-1].v)/4;
                    v.y = cells[i][j].v;
                } else {
                    //average velocity of surrounding cells to get the aproximate 2D velocity
                    v.y = (cells[i-1][j+1].v + cells[i-1][j-1].v + cells[i+1][j+1].v + cells[i+1][j-1].v)/4;
                    v.x = cells[i][j].v;
                }

                //now we can predict the previous position of the "particle"
                pos.x = cells[i][j].screen_pos.x - TIMESTEP*v.x; 
                pos.y = cells[i][j].screen_pos.y - TIMESTEP*v.y;

                //interpolate with nearby cells to work out the velocity of the particle
                w01 = ((float)((int)pos.x%GRID_SPACING_X)/(float)GRID_SPACING_X);
                w00 = 1.0f - w01;
                w11 = ((float)((int)pos.y%GRID_SPACING_Y)/(float)GRID_SPACING_Y);
                w10 = 1.0f - w11;
                
                if (cells[i-1][j+1].is_v_vertical){
                    u = cells[i-1][j-1].v*w00*w10 + cells[i+1][j-1].v*w01*w10 + cells[i-1][j+1].v*w01*w11 + cells[i+1][j+1].v*w00*w11;
                    //printf("%f\n", u);
                    cells[i][j].v = u*TIMESTEP*VEL_SCALE;
                }
                DrawCircleV(pos, 8, PARTICLE_COL);
            }
        }
    }

    public:
    GridPoint cells[GRID_WIDTH][GRID_HEIGHT];

    void apply_random_force() {
        for (int i = 0; i < GRID_WIDTH-1; i+=1) {
            for (int j = 1; j < GRID_HEIGHT-1; j+=2){
                cells[i][j].v += (rand() % 100) - 200;
            }
        }
    }

    void apply_mouse(){
        const int MOUSE_VEL = 1000;
        Vector2 mpos = GetMousePosition();
        if (IsKeyDown(KEY_A)) cells[(int)mpos.x/GRID_SPACING_X][(int)mpos.y/GRID_SPACING_Y].v -= MOUSE_VEL;
        if (IsKeyDown(KEY_D)) cells[(int)mpos.x/GRID_SPACING_X][(int)mpos.y/GRID_SPACING_Y].v += MOUSE_VEL;
    }

    void apple_edge_force(){
        for (int i = 0; i < GRID_HEIGHT-1; i++){
            cells[GRID_WIDTH-1][i].v += 100; 
        }
    }

    FluidGrid(bool edge_boundries) {
        //initialise all the grid cells
        for (int i = 0; i < GRID_WIDTH; i++) {
            for (int j = 0; j < GRID_HEIGHT; j++){
                cells[i][j].init((Vector2){(float)i, (float)j}, edge_boundries);
            }
        }
    }

    void draw_grid(bool draw_grid_lines, bool draw_arrows, bool draw_text) {
        //visualise each point sepatly to the cells
        for (int i = 0; i < GRID_WIDTH; i+=1) {
            for (int j = 0; j < GRID_HEIGHT; j+=1){
                cells[i][j].draw_grid(draw_grid_lines, draw_arrows, draw_text);
            }
        }
    }

    void update() {
        force_incompressable();
        advect_particles();
    }
};

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

bool use_edges = false;
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

    for (int i = 5; i < GRID_WIDTH-5; i+=1) {
        for (int j = 8; j < GRID_HEIGHT-8; j+=1){
            //grid.cells[i][j].s = 0;
            //grid.cells[i][j].v = 0;
        }
    }

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