#include <furi.h>
#include <gui/gui.h>

static volatile int exit_app;

static void my_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_draw_str(canvas, 20, 20, "Domo's Game of Life"); // Write on display

    canvas_draw_dot(canvas, 0, 0);
    canvas_draw_dot(canvas, 1, 1);
    canvas_draw_dot(canvas, 126, 62);
    canvas_draw_dot(canvas, 127, 63);

    canvas_draw_rframe(canvas, 0, 0, 10, 10, 2);
    canvas_draw_rframe(canvas, 59, 27, 10, 10, 2);
    canvas_draw_rframe(canvas, 118, 54, 10, 10, 2);
}

static void my_input_callback(InputEvent* input_event, void* context) {
    UNUSED(context);
    if(input_event->key == InputKeyBack) // Back key pressed?
        exit_app = 1;
}

int32_t domogol_app(void* p) {
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    void* my_context = NULL;
    UNUSED(p);

    // Prepare
    view_port_draw_callback_set(view_port, my_draw_callback, my_context);
    view_port_input_callback_set(view_port, my_input_callback, my_context);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Wait for back key
    while(exit_app != 1) {
        static int cnt;
        furi_delay_ms(1);
        cnt++;
        if(cnt >= 2000) break;
    }

    // Clean up
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);

    return 0;
}
