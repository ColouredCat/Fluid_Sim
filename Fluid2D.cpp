
#include "Fluid.h"

int get_uniform(const char* name, Shader s){
    int loc = GetShaderLocation(s, name);
    if (loc == -1) { 
        printf("Could not find uniform \"%s\"\n", name);
        return -1;
    }
    return loc;
}

void set_uniform(int loc, float value, Shader s){
    const float valc[1] = {value};
    SetShaderValue(s, loc, valc, SHADER_ATTRIB_FLOAT);
}

void  GridPoint::init(Vector2 position, bool edge_boundries) {
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

void  GridPoint::draw_grid(bool draw_grid_lines, bool draw_arrows, bool draw_text){
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

void FluidGrid::force_incompressable() {
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

void FluidGrid::draw_particle(Vector2 pos, float v) {
    DrawCircleV(pos, 5, PARTICLE_COL);
    if(use_shaders) set_uniform(loc, v, s);
}

void FluidGrid::advect_particles() {
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
            draw_particle(pos, u);
        }
    }
}

void FluidGrid::apply_random_force() {
    for (int i = 0; i < GRID_WIDTH-1; i+=1) {
        for (int j = 1; j < GRID_HEIGHT-1; j+=1){
            cells[i][j].v += (rand() % 100) - 200;
        }
    }
}

void FluidGrid::apply_mouse(){
    const int MOUSE_VEL = 1000;
    Vector2 mpos = GetMousePosition();
    if (IsKeyDown(KEY_A)) cells[(int)mpos.x/GRID_SPACING_X][(int)mpos.y/GRID_SPACING_Y].v -= MOUSE_VEL;
    if (IsKeyDown(KEY_D)) cells[(int)mpos.x/GRID_SPACING_X][(int)mpos.y/GRID_SPACING_Y].v += MOUSE_VEL;
}

void FluidGrid::apple_edge_force(){
    for (int i = 0; i < GRID_HEIGHT-1; i++){
        cells[GRID_WIDTH-1][i].v += 100; 
    }
}

void FluidGrid::reset(){
    //initialise all the grid cells
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++){
            cells[i][j].init((Vector2){(float)i, (float)j}, true);
        }
    }
}

void FluidGrid::init(const bool edge_boundries, const char* fragment_shader, const char* vertex_shader){ 
    //setup shader
    s = LoadShader(vertex_shader, fragment_shader);
    if (!IsShaderValid(s)) { 
        printf("Failed to load shaders: %s, %s", fragment_shader, vertex_shader);
        use_shaders = false;
    } else {
        int x = get_uniform("u_width", s);
        int y = get_uniform("u_height", s);
        set_uniform(x, WIDTH, s);
        set_uniform(y, HEIGHT, s);
    }
    loc = get_uniform("u_scale", s);
    //initialise all the grid cells
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++){
            cells[i][j].init((Vector2){(float)i, (float)j}, edge_boundries);
        }
    }
}

void FluidGrid::draw_grid(bool draw_grid_lines, bool draw_arrows, bool draw_text) {
    //visualise each point sepatly to the cells
    for (int i = 0; i < GRID_WIDTH; i+=1) {
        for (int j = 0; j < GRID_HEIGHT; j+=1){
            cells[i][j].draw_grid(draw_grid_lines, draw_arrows, draw_text);
        }
    }
}

void FluidGrid::update() {
    if(use_shaders) BeginShaderMode(s);
    force_incompressable();
    advect_particles();
    if(use_shaders) EndShaderMode();
}