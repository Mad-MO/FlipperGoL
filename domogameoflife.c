#include <furi.h>
#include <gui/gui.h>

static volatile int exit_app;

static void my_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_draw_str(canvas, 10, 10, "Game of Life"); // Write on display
}

static void my_input_callback(InputEvent* input_event, void* context) {
    UNUSED(context);
    if(input_event->key == InputKeyBack) // Back key pressed?
        exit_app = 1;
}

int32_t domogameoflife_app(void* p) {
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    void* my_context = NULL;
    UNUSED(p);

    // Prepare
    view_port_draw_callback_set(view_port, my_draw_callback, my_context);
    view_port_input_callback_set(view_port, my_input_callback, my_context);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Wait for back key
    while(exit_app != 1)
        ;

    // Clean up
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);

    return 0;
}
