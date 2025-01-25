#include <furi.h>
#include <gui/gui.h>

#define WIDTH    128
#define HEIGHT   64

static volatile int exit_app;
static uint8_t grid[WIDTH][HEIGHT];
static uint8_t new_grid[WIDTH][HEIGHT];
static uint8_t fullscreen;
static uint8_t speed;
static uint16_t cells;
static uint32_t cycles;
static uint8_t mode;



void init_grid(void)
{
    int x, y;
    for(x=0; x<WIDTH; x++)
    {
        for(y=0; y<HEIGHT; y++)
        {
            if(random() & 1)
            {
                grid[x][y] = 1;
            }
            else
            {
                grid[x][y] = 0;
            }
        }
    }
    cycles = 0;
}



static void draw_grid_callback(Canvas* canvas, void* context)
{
    int x, y;
    char str[16];
    UNUSED(context);

    canvas_clear(canvas); // Clear display
    for(x=0; x<WIDTH; x++)
    {
        for(y=0; y<HEIGHT; y++)
        {
            if(grid[x][y])
            {
                canvas_draw_dot(canvas, x, y);
            }
        }
    }

    // Handle status line
    if(!fullscreen)
    {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 0, 54, 128, 10);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_line(canvas, 0, 53, 127, 53);
        snprintf(str, sizeof(str), "Cyc:%li", cycles);
        canvas_draw_str(canvas, 0, 62, str);
        snprintf(str, sizeof(str), "Cell:%i", cells);
        canvas_draw_str(canvas, 56, 62, str);
        snprintf(str, sizeof(str), "Spd:%i", speed);
        canvas_draw_str(canvas, 105, 62, str);
    }
}



void update_grid(void)
{
    int x, y;
    cycles++;
    cells = 0;
    for(x=0; x<WIDTH; x++)
    {
        for(y=0; y<HEIGHT; y++)
        {
            int neighbors = 0;
            for(int dx=-1; dx<=1; dx++)
            {
                for(int dy=-1; dy<=1; dy++)
                {
                    if(dx == 0 && dy == 0)
                    {
                        continue;
                    }
                    int nx = (x + dx) % WIDTH;
                    int ny = (y + dy) % HEIGHT;
                    neighbors += grid[nx][ny];
                }
            }
            if     ((grid[x][y] == 1) && (neighbors  < 2)) // Underpopulation
            {
                new_grid[x][y] = 0;
            }
            else if((grid[x][y] == 1) && (neighbors  > 3)) // Overpopulation
            {
                new_grid[x][y] = 0;
            }
            else if((grid[x][y] == 0) && (neighbors == 3)) // Reproduction
            {
                new_grid[x][y] = 1;
                cells++;
            }
            else                                           // Stasis
            {
                new_grid[x][y] = grid[x][y];
                if(new_grid[x][y] == 1)
                {
                    cells++;
                }
            }
        }
    }

    memcpy(grid, new_grid, sizeof(grid));
}



static void input_callback(InputEvent* input_event, void* context)
{
    UNUSED(context);
    if      (input_event->key == InputKeyBack) // Back key pressed?
    {
        exit_app = 1;
    }
    else if((input_event->key == InputKeyOk)    && (input_event->type == InputTypeRelease))
    {
        fullscreen++;
        fullscreen %= 2;
    }
    else if((input_event->key == InputKeyUp)    && (input_event->type == InputTypeRelease))
    {
        if(speed < 9) speed++;
    }
    else if((input_event->key == InputKeyDown)  && (input_event->type == InputTypeRelease))
    {
        if(speed > 0)
        {
            speed--;
        }
    }
    else if((input_event->key == InputKeyRight) && (input_event->type == InputTypeRelease))
    {
        mode = 1;
    }
    else if((input_event->key == InputKeyLeft)  && (input_event->type == InputTypeRelease))
    {
        mode = 1;
    }
}



int32_t flippergol_app(void* p)
{
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    void* my_context = NULL;
    UNUSED(p);

    // Init grid
    init_grid();
    speed = 5;

    // Prepare
    view_port_draw_callback_set(view_port, draw_grid_callback, my_context);
    view_port_input_callback_set(view_port, input_callback, my_context);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Loop until back key is pressed
    while(exit_app != 1)
    {
        // Handle speed
        furi_delay_ms(50 * (10 - speed));

        // Handle different modes and update grid
        if(mode)
        {
            init_grid();
            mode = 0;
        }
        else
        {
            update_grid();
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
