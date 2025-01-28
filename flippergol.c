
// File:    flippergol.c
// Author:  Martin Ochs
// License: MIT
// Brief:   A very basic python implementation for Conway's "Game of Life"
//          using Tkinter for the GUI. The simulation can be controlled by
//          the speed slider and the buttons for different initial states.



#include <furi.h>
#include <gui/gui.h>
#include "flippergol_icons.h"



// Define SW name and Version
#define SW_NAME "Game of Life"
#define SW_VERS "v0.3"

// Define the size of the grid
#define GRID_WIDTH    128
#define GRID_HEIGHT   64

// Create the grid to represent the cells
static volatile uint8_t exit_app;
static uint8_t grid[GRID_WIDTH][GRID_HEIGHT];
static uint8_t new_grid[GRID_WIDTH][GRID_HEIGHT];
static uint8_t fullscreen;
static uint8_t speed;
static uint16_t cells_alive;
static uint32_t cycle_counter;
#define END_DET_CNT 60
static uint8_t end_det[END_DET_CNT];
static uint8_t end_det_pos;


typedef enum
{
    ModeTypeRandom,
    ModeTypeBlinker,
    ModeTypeGlider,
    ModeTypeGliderGun,
    ModeTypePentomino,
    ModeTypeDiehard,
    ModeTypeAcorn,
    // ----------------
    ModeTypeMax
} ModeType;
static ModeType mode;

typedef enum
{
    StageTypeStartup,  // Show startup screen
    StageTypeInit,     // Initialize grid
    StageTypeShowInfo, // Show info for current mode
    StageTypeRunning,  // Running simulation
    StageTypeEnd       // End of simulation has been reached
} StageType;
static StageType stage;



// Function to initialize the grid
void init_grid(void)
{
    memset(grid, 0, sizeof(grid));

    if(mode == ModeTypeRandom)
    {
        uint8_t x, y;
        for(x=0; x<GRID_WIDTH; x++)
            for(y=0; y<GRID_HEIGHT; y++)
                grid[x][y] = (random() & 1);
    }
    else if(mode == ModeTypeBlinker)
    {
        grid[1+(GRID_WIDTH/2)][0+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][1+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][2+(GRID_HEIGHT/2)] = 1;
    }
    else if(mode == ModeTypeGlider)
    {
        grid[0+(GRID_WIDTH/2)][2+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][0+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][2+(GRID_HEIGHT/2)] = 1;
        grid[2+(GRID_WIDTH/2)][1+(GRID_HEIGHT/2)] = 1;
        grid[2+(GRID_WIDTH/2)][2+(GRID_HEIGHT/2)] = 1;
    }
    else if(mode == ModeTypeGliderGun)
    {
        grid[ 1][5] = 1; grid[ 1][6] = 1;
        grid[ 2][5] = 1; grid[ 2][6] = 1;
        grid[11][5] = 1; grid[11][6] = 1; grid[11][7] = 1;
        grid[12][4] = 1; grid[12][8] = 1;
        grid[13][3] = 1; grid[13][9] = 1;
        grid[14][3] = 1; grid[14][9] = 1;
        grid[15][6] = 1;
        grid[16][4] = 1; grid[16][8] = 1;
        grid[17][5] = 1; grid[17][6] = 1; grid[17][7] = 1;
        grid[18][6] = 1;
        grid[21][3] = 1; grid[21][4] = 1; grid[21][5] = 1;
        grid[22][3] = 1; grid[22][4] = 1; grid[22][5] = 1;
        grid[23][2] = 1; grid[23][6] = 1;
        grid[25][1] = 1; grid[25][2] = 1; grid[25][6] = 1; grid[25][7] = 1;
        grid[35][3] = 1; grid[35][4] = 1;
        grid[36][3] = 1; grid[36][4] = 1;
    }
    else if(mode == ModeTypePentomino)
    {
        grid[0+(GRID_WIDTH/2)][1+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][0+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][1+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][2+(GRID_HEIGHT/2)] = 1;
        grid[2+(GRID_WIDTH/2)][0+(GRID_HEIGHT/2)] = 1;
    }
    else if(mode == ModeTypeDiehard)
    {
        grid[0+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][5+(GRID_HEIGHT/2)] = 1;
        grid[5+(GRID_WIDTH/2)][5+(GRID_HEIGHT/2)] = 1;
        grid[6+(GRID_WIDTH/2)][3+(GRID_HEIGHT/2)] = 1;
        grid[6+(GRID_WIDTH/2)][5+(GRID_HEIGHT/2)] = 1;
        grid[7+(GRID_WIDTH/2)][5+(GRID_HEIGHT/2)] = 1;
    }
    else if(mode == ModeTypeAcorn)
    {
        grid[0+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][2+(GRID_HEIGHT/2)] = 1;
        grid[1+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
        grid[3+(GRID_WIDTH/2)][3+(GRID_HEIGHT/2)] = 1;
        grid[4+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
        grid[5+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
        grid[6+(GRID_WIDTH/2)][4+(GRID_HEIGHT/2)] = 1;
    }

    cycle_counter = 0;
}



// Function to update the grid based on the game of life rules
void update_grid(void)
{
    uint8_t x, y;
    for(x=0; x<GRID_WIDTH; x++)
    {
        for(y=0; y<GRID_HEIGHT; y++)
        {
            uint8_t neighbors = 0;
            for(int8_t dx=-1; dx<=1; dx++)
            {
                for(int8_t dy=-1; dy<=1; dy++)
                {
                    if(dx == 0 && dy == 0)
                    {
                        continue;
                    }
                    uint8_t nx = (GRID_WIDTH + x + dx) % GRID_WIDTH;
                    uint8_t ny = (GRID_HEIGHT + y + dy) % GRID_HEIGHT;
                    neighbors += grid[nx][ny];
                }
            }

            if     ((grid[x][y] == 1) && (neighbors  < 2)) // Underpopulation
                new_grid[x][y] = 0;
            else if((grid[x][y] == 1) && (neighbors  > 3)) // Overpopulation
                new_grid[x][y] = 0;
            else if((grid[x][y] == 0) && (neighbors == 3)) // Reproduction
                new_grid[x][y] = 1;
            else                                           // Stasis
                new_grid[x][y] = grid[x][y];
        }
    }

    memcpy(grid, new_grid, sizeof(grid));
}



// Function to draw a string in a rounded frame for the mode name
static void draw_str_in_rounded_frame(Canvas* canvas, const char* str)
{
    uint8_t width;
    width = canvas_string_width(canvas, str);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 59-width/2, 11, width+6, 15);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 60-width/2, 12, width+4, 13, 2);
    canvas_draw_str(canvas, 62-width/2, 21, str);
}



// Function to draw the grid on the canvas
static void draw_grid_callback(Canvas* canvas, void* context)
{
    uint8_t x, y;
    char str[16];
    UNUSED(context);

    // Clear display
    canvas_clear(canvas);
    cells_alive = 0;

    // Draw grid to canvas
    if(stage >= StageTypeInit)
    {
        for(x=0; x<GRID_WIDTH; x++)
        {
            for(y=0; y<GRID_HEIGHT; y++)
            {
                if(grid[x][y])
                {
                    canvas_draw_dot(canvas, x, y);
                    cells_alive++;
                }
            }
        }
    }

    // Handle startup screen
    if(stage == StageTypeStartup)
    {
        canvas_draw_icon(canvas, 0,  0, &I_flippergol);
        canvas_draw_str(canvas, 14, 8, SW_NAME);         canvas_draw_str(canvas, 70,  8, SW_VERS "  (domo)");
        canvas_draw_str(canvas, 14, 21, "Return:");      canvas_draw_str(canvas, 70, 21, "End program");
        canvas_draw_str(canvas, 14, 31, "OK:");          canvas_draw_str(canvas, 70, 31, "Fullscreen");
        canvas_draw_str(canvas, 14, 41, "Right, left:"); canvas_draw_str(canvas, 70, 41, "Init modes");
        canvas_draw_str(canvas, 14, 51, "Up, down:");    canvas_draw_str(canvas, 70, 51, "Adjust Speed");
        canvas_draw_str(canvas,  0, 64, "Press OK to start");
    }

    // Handle mode info
    if(stage == StageTypeShowInfo)
    {
        if     (mode == ModeTypeRandom)
            draw_str_in_rounded_frame(canvas, "Random");
        else if(mode == ModeTypeBlinker)
            draw_str_in_rounded_frame(canvas, "Blinker");
        else if(mode == ModeTypeGlider)
            draw_str_in_rounded_frame(canvas, "Glider");
        else if(mode == ModeTypeGliderGun)
            draw_str_in_rounded_frame(canvas, "Glider gun");
        else if(mode == ModeTypePentomino)
            draw_str_in_rounded_frame(canvas, "Pentomino");
        else if(mode == ModeTypeDiehard)
            draw_str_in_rounded_frame(canvas, "Diehard");
        else if(mode == ModeTypeAcorn)
            draw_str_in_rounded_frame(canvas, "Acorn");
    }

    // Handle status line
    if(    (stage >= StageTypeInit)
        && (!fullscreen)
      )
    {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 0, 54, 128, 10);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_line(canvas, 0, 53, 127, 53);
        snprintf(str, sizeof(str), "Cyc:%li", cycle_counter);
        canvas_draw_str(canvas, 0, 62, str);
        snprintf(str, sizeof(str), "Cell:%i", cells_alive);
        canvas_draw_str(canvas, 56, 62, str);
        snprintf(str, sizeof(str), "Spd:%i", speed);
        canvas_draw_str(canvas, 105, 62, str);
    }
}



// Function to handle input events
static void input_callback(InputEvent* input_event, void* context)
{
    UNUSED(context);
    if      (input_event->key == InputKeyBack) // Back key pressed?
    {
        exit_app = 1;
    }
    else if(    (input_event->key  == InputKeyOk)
             && (input_event->type == InputTypeRelease)
           )
    {
        if(stage == StageTypeStartup)
        {
            stage = StageTypeInit;
        }
        else
        {
            fullscreen++;
            fullscreen %= 2;
        }
    }
    else if(    (input_event->key  == InputKeyUp)
             && (input_event->type == InputTypeRelease)
           )
    {
        if(speed < 5) speed++;
    }
    else if(    (input_event->key  == InputKeyDown)
             && (input_event->type == InputTypeRelease)
           )
    {
        if(speed > 1) speed--;
    }
    else if(    (input_event->key  == InputKeyRight)
             && (input_event->type == InputTypeRelease)
           )
    {
        mode++;
        mode %= ModeTypeMax;
        stage = StageTypeInit;
    }
    else if(    (input_event->key  == InputKeyLeft)
             && (input_event->type == InputTypeRelease)
           )
    {
        if(mode == 0) mode = ModeTypeMax - 1;
        else          mode--;
        stage = StageTypeInit;
    }
}



uint8_t end_detection(void)
{
    end_det_pos++;
    end_det_pos %= END_DET_CNT;
    end_det[end_det_pos] = cells_alive;
    if(cells_alive == 0)
        return 1;
    if(cycle_counter > END_DET_CNT)                                 // At least END_DET_CNT cycles needed for detection
    {
        for(uint8_t pattern=1; pattern<=(END_DET_CNT/2); pattern++) // Test pattern in the length of 1 to half of the buffer
        {
            for(uint8_t testpos=pattern; testpos<END_DET_CNT; testpos++)
            {
                if(end_det[testpos] != end_det[testpos % pattern])  // Pattern not found? -> End loop and test next pattern
                    break;
                if(testpos == END_DET_CNT - 1)                      // End of loop reached? -> Pattern found!
                    return 1;
            }
        }
    }
    return 0;
}



// Function to handle one life cycle of the simulation
int32_t flippergol_app(void* p)
{
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    void* my_context = NULL;
    UNUSED(p);

    // Prepare
    view_port_draw_callback_set(view_port, draw_grid_callback, my_context);
    view_port_input_callback_set(view_port, input_callback, my_context);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Init
    static uint32_t last_ticks;
    last_ticks = furi_get_tick();
    mode  = ModeTypeRandom;
    stage = StageTypeStartup;
    speed = 5;

    // Loop until back key is pressed
    while(exit_app != 1)
    {
        static uint32_t timer;
               uint32_t ticks;

        // Handle passed time
        ticks = furi_get_tick();
        timer += (ticks - last_ticks);
        last_ticks = ticks;

        // Handle speed
        if     (speed == 1) furi_delay_ms(500);
        else if(speed == 2) furi_delay_ms(250);
        else if(speed == 3) furi_delay_ms(125);
        else if(speed == 4) furi_delay_ms( 50);
        else                furi_delay_ms( 10);

        // Handle different modes and update grid
        if     (stage == StageTypeStartup)
        {
            if(timer >= 20000)
            {
                stage = StageTypeInit;
                timer = 0;
            }
        }
        else if(stage == StageTypeInit)
        {
            init_grid();
            stage = StageTypeShowInfo;
            timer = 0;
        }
        else if(stage == StageTypeShowInfo)
        {
            if(timer >= 2000)
            {
                stage = StageTypeRunning;
                timer = 0;
            }
        }
        else if(stage == StageTypeRunning)
        {
            if(end_detection())
            {
                stage = StageTypeEnd;
                timer = 0;
            }
            else
            {
                cycle_counter++;
                update_grid();
            }
        }
        else if(stage == StageTypeEnd)
        {
            update_grid();
            if(timer >= 5000)
            {
                mode++;
                mode %= ModeTypeMax;
                stage = StageTypeInit;
                timer = 0;
            }
        }

        // Update canvas
        view_port_update(view_port);
    }

    // Clean up
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);

    return 0;
}
