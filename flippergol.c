#include <furi.h>
#include <gui/gui.h>

#define WIDTH    32 // 128
#define HEIGHT   32 // 64

static volatile int exit_app;
static uint8_t grid[WIDTH*HEIGHT/8];
static uint8_t new_grid[WIDTH*HEIGHT/8];



#define SET_ARRAY_BIT(array, x, y)   array[((x*WIDTH)+y)/8] |=  (1 << (((x*WIDTH)+y)%8))
#define CLR_ARRAY_BIT(array, x, y)   array[((x*WIDTH)+y)/8] &= ~(1 << (((x*WIDTH)+y)%8))
#define GET_ARRAY_BIT(array, x, y) ((array[((x*WIDTH)+y)/8] &   (1 << (((x*WIDTH)+y)%8))) ? 1 : 0)



void init_grid(void)
{
    int x, y;
    for(x=0; x<WIDTH; x++)
    {
        for(y=0; y<HEIGHT; y++)
        {
            if(random() & 1)
            {
                SET_ARRAY_BIT(grid, x, y);
            }
            else
            {
                CLR_ARRAY_BIT(grid, x, y);
            }
if(x==y) SET_ARRAY_BIT(grid, x, y);
if(x==0) SET_ARRAY_BIT(grid, x, y);
if(y==0) SET_ARRAY_BIT(grid, x, y);
        }
    }
}



static void draw_grid_callback(Canvas* canvas, void* context)
{
    int x, y;
    UNUSED(context);

    canvas_clear(canvas); // Clear display
    for(x=0; x<WIDTH; x++)
    {
        for(y=0; y<HEIGHT; y++)
        {
            if(GET_ARRAY_BIT(grid, x, y))
            {
                canvas_draw_dot(canvas, x, y);
            }
        }
    }
}



void update_grid(void)
{
    int x, y;

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
                    neighbors += GET_ARRAY_BIT(grid, nx, ny);
                }
            }
            if     ((GET_ARRAY_BIT(grid, x, y) == 1) && (neighbors  < 2)) // Underpopulation
            {
                CLR_ARRAY_BIT(new_grid, x, y);
            }
            else if((GET_ARRAY_BIT(grid, x, y) == 1) && (neighbors  > 3)) // Overpopulation
            {
                CLR_ARRAY_BIT(new_grid, x, y);
            }
            else if((GET_ARRAY_BIT(grid, x, y) == 0) && (neighbors == 3)) // Reproduction
            {
                SET_ARRAY_BIT(new_grid, x, y);
            }
            else                                           // Stasis
            {
                if(GET_ARRAY_BIT(grid, x, y))
                {
                    SET_ARRAY_BIT(new_grid, x, y);
                }
                else
                {
                    CLR_ARRAY_BIT(new_grid, x, y);
                }
            }
        }

    }

    memcpy(grid, new_grid, sizeof(grid));
}



static void input_callback(InputEvent* input_event, void* context)
{
    UNUSED(context);
    if(input_event->key == InputKeyBack) // Back key pressed?
        exit_app = 1;
}



int32_t flippergol_app(void* p)
{
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    void* my_context = NULL;
    UNUSED(p);

    // Init grid
    init_grid();

    // Prepare
    view_port_draw_callback_set(view_port, draw_grid_callback, my_context);
    view_port_input_callback_set(view_port, input_callback, my_context);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Wait for back key
    while(exit_app != 1)
    {
        furi_delay_ms(50);
        update_grid();
        view_port_update(view_port);
    }

    // Clean up
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);

    return 0;
}
