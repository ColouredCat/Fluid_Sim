
#ifndef FLUID
#define FLUID
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <stdbool.h>
#include <time.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;
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

    void init(Vector2 position, bool edge_boundries);

    void draw_grid(bool draw_grid_lines, bool draw_arrows, bool draw_text);
};

class FluidGrid {
    private:
    void force_incompressable() ;
    void advect_particles() ;

    public:
    GridPoint cells[GRID_WIDTH][GRID_HEIGHT];

    void apply_random_force() ;
    void apply_mouse() ;
    void apple_edge_force() ;

    FluidGrid(bool edge_boundries) ;
    void draw_grid(bool draw_grid_lines, bool draw_arrows, bool draw_text) ;
    void update() ;
};
#endif
