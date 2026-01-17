
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <stdbool.h>
#include <time.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;
const char* WIN_NAME = "Fluid Sim";
const int FPS = 60;
const float TIMESTEP = 1/(float)FPS;

const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;
const int GRID_SPACING_X = (WIDTH / GRID_WIDTH);
const int GRID_SPACING_Y = (HEIGHT / GRID_HEIGHT);

const float OVERRELAX = 1.8f;
const int ITERATIONS = 30;
const float GRAVITY = 9.81f;

const int POINT_SIZE = 5;
const Color LINE_COL = GRAY;
const Color CELL_COL = BLUE;
const Color ARROW_COL = RED;

class GridPoint {
    public:   
    Vector2 grid_pos;
    Vector2 screen_pos;
    bool is_v_vertical;

    float v = 0;
    float d = 0;
    float s = 1;

    void init(Vector2 position) {
        //calculate the cell's position on screen from it's array position
        grid_pos = position;
        screen_pos.x = (GRID_SPACING_X * position.x);
        screen_pos.y = (GRID_SPACING_Y * position.y);
        v = (rand() % 200) - 100;
        //cells on every second row show vertical velocity
        is_v_vertical = ((int)position.y % 2);
        if (is_v_vertical) screen_pos.x += GRID_SPACING_X/2;
        //mark cells on the outside as bariers with no velocity
        if ((grid_pos.x == 0 || grid_pos.x == GRID_WIDTH-1) || (grid_pos.y == 0 || grid_pos.y == GRID_HEIGHT-1)){
            s = 0;
            v = 0;
        }
    }

    void draw(bool draw_grid_lines){
        //only draw divergence for cells not on the edge
        if (s != 0){
            //DrawRectangle(screen_pos.x, screen_pos.y, GRID_SPACING_X, GRID_SPACING_Y, (Color){v*5, 0, 0, 255});
            char str[20];
            sprintf(str, "%f", v);
            //DrawText(str, screen_pos.x + 10, screen_pos.y + 10, 10, GRAY);
        }
        //draw horizontal and vertical grid lines only on the outer rows
        //if(draw_grid_lines && grid_pos.x == 0 && is_v_vertical) DrawLine(0, screen_pos.y, WIDTH, screen_pos.y, LINE_COL);
        //if(draw_grid_lines && grid_pos.y == 0) DrawLine(screen_pos.x, 0, screen_pos.x, HEIGHT, LINE_COL);
        //DrawCircleV(screen_pos, POINT_SIZE, CELL_COL);
        //if (is_v_vertical) DrawLineV(screen_pos, (Vector2){screen_pos.x, screen_pos.y + v}, ARROW_COL);
        //if (!is_v_vertical) DrawLineV(screen_pos, (Vector2){screen_pos.x + v, screen_pos.y}, ARROW_COL);
    }
};

class FluidGrid {
    private:

    void apply_forces() {
        for (int i = 1; i < GRID_WIDTH-1; i+=1) {
            for (int j = 2; j < GRID_HEIGHT-1; j+=2){
                cells[i][j].v += (rand() % 200) - 400;
                //cells[i][j].v += GRAVITY*TIMESTEP;
            }
        }
    }

    void force_incompressable() {
        float d, s;
        //update all cells, ignoring the outermost ones
        for (int i = 1; i < GRID_WIDTH-1; i+=1) {
            for (int j = 2; j < GRID_HEIGHT-1; j+=2){
                //step 1 : force incompressability
                //calculate divergance, i.e whether the cell has fluid flowing in or out of it, or if it is incompressible
                d = OVERRELAX * (cells[i+1][j].v - cells[i][j].v + cells[i][j+1].v - cells[i][j-1].v);
                cells[i][j].d = d;
                s = cells[i+1][j].s + cells[i][j].s + cells[i][j+1].s + cells[i][j-1].s;
                //update v values based on this
                cells[i][j].v += d*cells[i][j].s/s;
                cells[i][j+1].v -= d*cells[i][j+1].s/s;
                cells[i+1][j].v -= d*cells[i+1][j].s/s;
                cells[i][j-1].v += d*cells[i][j-1].s/s;
            }
        }
    }

    void advect_particles() {
        Vector2 v, u;
        Vector2 pos;
        float w00, w01, w10, w11;
        for (int i = 1; i < GRID_WIDTH-1; i++){
            for (int j = 2; j < GRID_HEIGHT-1; j+=2) {
                //average velocity of surrounding cells to get the aproximate 2D velocity
                v.y = (cells[i-1][j+1].v + cells[i-1][j-1].v + cells[i+1][j+1].v + cells[i+1][j-1].v)/4;
                v.x = cells[i][j].v;
                //now we can predict the previous position of the "particle"
                pos.x = cells[i][j].screen_pos.x - TIMESTEP*v.x; 
                pos.y = cells[i][j].screen_pos.y - TIMESTEP*v.y;
                DrawCircleV(pos, 5, GREEN);
                //DrawLineV(pos, cells[i][j].screen_pos, YELLOW);
                //
                w00 = 1 / (((int)pos.x%GRID_SPACING_X)/GRID_SPACING_X);
                w01 = (((int)pos.x%GRID_SPACING_X)/GRID_SPACING_X);
                w10 = 1 / (((int)pos.y%GRID_SPACING_Y)/GRID_SPACING_Y);
                w11 = (((int)pos.y%GRID_SPACING_Y)/GRID_SPACING_Y);
                u.y = cells[i-1][j-1].v*w00*w10 + cells[i+1][j-1].v*w01*w10 + cells[i-1][j+1].v*w01*w11 + cells[i+1][j+1].v*w00*w11;
                //cells[i][j].v = u.y;
            }
        }
    }

    public:
    GridPoint cells[GRID_WIDTH][GRID_HEIGHT];

    FluidGrid() {
        //dynamically alocate memory as the grid too huge for the stack :(
        //cells = malloc(sizeof(GridPoint[GRID_WIDTH][GRID_HEIGHT]));
        //initialise all the grid cells
        for (int i = 0; i < GRID_WIDTH; i++) {
            for (int j = 0; j < GRID_HEIGHT; j++){
                cells[i][j].init((Vector2){(float)i, (float)j});
            }
        }
    }

    void draw(bool draw_grid_lines) {
        //visualise each point sepatly to the cells
        for (int i = 0; i < GRID_WIDTH; i+=1) {
            for (int j = 0; j < GRID_HEIGHT; j+=1){
                cells[i][j].draw(draw_grid_lines);
            }
        }
    }

    void update() {
        apply_forces();
        for (int i = 0; i < ITERATIONS; i++) {
            force_incompressable();
        }
        advect_particles();
    }
};

int main() {
    InitWindow(WIDTH, HEIGHT, WIN_NAME);
    ToggleFullscreen();
    SetTargetFPS(FPS);
    srand(clock());
    FluidGrid grid;

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        grid.draw(true);
        grid.update();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}