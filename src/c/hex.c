#include <pebble.h>

#define TEXT_LAYER_DATE     0
#define TEXT_LAYER_WEEKDAY  1
#define TEXT_LAYER_TIME     2
#define TEXT_LAYER_RFFU     3
#define TEXT_LAYER_STAT     4


static Window *s_main_window;
static GFont s_font;
static GFont s_time_font;

static TextLayer *s_hex_layers[5];
static TextLayer *s_time_layer;

// TODO: change colors - from settings panel
static GColor s_main_color = GColorWhite;
static GColor s_accent_color = GColorCyan;
static GColor s_background_color = GColorBlack;

static int s_hours;
static int s_minutes;
static int s_seconds;

static int s_year;
static int s_month;
static int s_day;

static int s_weekday;
static int s_connected;
static int s_battery_level;

static int randoms[16];
static char *s_buffers[6];
static char s_time_buffer[15];

// render selected text layer
static void render_row(int row) {
    if (row == TEXT_LAYER_TIME) {
        snprintf(s_buffers[row], 15, "%02X          %02X", randoms[6], randoms[7]);
    } else if (row == TEXT_LAYER_DATE) {
        snprintf(s_buffers[row], 15, "%02X %02X %02X %02X %02X", randoms[0], s_day, s_month, s_year, randoms[1]);
    } else if (row == TEXT_LAYER_WEEKDAY) {
        snprintf(s_buffers[row], 15, "%02X %02X %02X %02X %02X", randoms[2], randoms[3], s_weekday, randoms[4], randoms[5]);
    } else if (row == TEXT_LAYER_RFFU) {
        snprintf(s_buffers[row], 15, "%02X %02X %02X %02X %02X", randoms[8], randoms[9], randoms[10], randoms[11], randoms[12]);
    } else if (row == TEXT_LAYER_STAT) {
        snprintf(s_buffers[row], 15, "%02X %02X %02X %02X %02X", randoms[13], s_connected, randoms[14], s_battery_level, randoms[15]);
    }

    if (row >= 0 && row < 6) {
        text_layer_set_text(s_hex_layers[row], s_buffers[row]);
    }
}

static void render_time() {
    snprintf(s_time_buffer, 15, "   %02X %02X %02X   ", s_hours, s_minutes, s_seconds);
    text_layer_set_text(s_time_layer, s_time_buffer);
}

// update time
static void update_time(bool init) {
    // get time
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // get seconds as int
    static char s_second_buffer[8];
    strftime(s_second_buffer, sizeof(s_second_buffer), "%S", tick_time);
    s_seconds = atoi(s_second_buffer);

    if (init) {
        srand(temp);
    }
    
    // generate randoms
    if (s_seconds == 0 || init) {
        for (int i = 0; i < 16; ++i) {
            randoms[i] = rand() % 256;
        } 
    }

    if (s_seconds == 0 || init) { // EOM
        // get minutes as int
        static char s_minute_buffer[8];
        strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
        s_minutes = atoi(s_minute_buffer);

        if (s_minutes == 0 || init) { // EOH
            // get hours as int
            static char s_hour_buffer[8];
            strftime(s_hour_buffer, sizeof(s_hour_buffer), "%H", tick_time);
            s_hours = atoi(s_hour_buffer);

            if (s_hours == 0 || init) { // EOD
                // get day as int
                static char s_day_buffer[8];
                strftime(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time);
                s_day = atoi(s_day_buffer);

                // get weekday as int
                static char s_weekday_buffer[8];
                strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%w", tick_time);
                s_weekday = ((atoi(s_weekday_buffer) + 6) % 7) + 1;

                // get month as int
                static char s_month_buffer[8];
                strftime(s_month_buffer, sizeof(s_month_buffer), "%m", tick_time);
                s_month = atoi(s_month_buffer);

                // get year as int
                static char s_year_buffer[8];
                strftime(s_year_buffer, sizeof(s_year_buffer), "%y", tick_time);
                s_year = atoi(s_year_buffer);

                // display updated date
                render_row(TEXT_LAYER_DATE);
                render_row(TEXT_LAYER_WEEKDAY);
            }
        }
    }
    
    // display updated time
    render_time();

    if (s_seconds == 0 || init) {
        if (s_hours != 0) {
            render_row(TEXT_LAYER_DATE);
            render_row(TEXT_LAYER_WEEKDAY);
        }
        render_row(TEXT_LAYER_TIME);
        render_row(TEXT_LAYER_RFFU);
        render_row(TEXT_LAYER_STAT);
    }
}

// callback for the time service
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(false);
}

// update battery level
static void battery_callback(BatteryChargeState state) {
    if (s_battery_level != state.charge_percent) {
        s_battery_level = state.charge_percent;
        render_row(TEXT_LAYER_STAT);
    }
}

// update bt connection status
static void connection_callback(bool connected) {
    s_connected = connected ? 255 : 0;
    render_row(TEXT_LAYER_STAT);
}

//TODO: load and deploy settings

// initialize the different layers when the window is created
static void main_window_load(Window *window) {
    // get root layer and boundaries of the watch
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // set the background color for the root window
    window_set_background_color(s_main_window, s_background_color);

    // get font
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINESS_20));
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINESS_BOLD_20));

    // create text layers
    for (int i = 0; i < 5; ++i) {
        TextLayer *layer = s_hex_layers[i] = text_layer_create(GRect(0, 8 + (i * 32), bounds.size.w, 20));

        // set options for text layer
        text_layer_set_background_color(layer, GColorClear);
        text_layer_set_text_color(layer, s_accent_color);
        text_layer_set_font(layer, s_font);
        text_layer_set_text_alignment(layer, GTextAlignmentCenter);
    }

    // create time text layer
    s_time_layer = text_layer_create(GRect(0, 72, bounds.size.w, 22));

    // set options for the time text layer
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, s_main_color);
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    // add the layers to the main window
    for (int i = 0; i < 5; ++i) {
        layer_add_child(window_layer, text_layer_get_layer(s_hex_layers[i]));
    }
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

// free layers on destroy
static void main_window_unload(Window *window) {
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    connection_service_unsubscribe();

    for (int i = 0; i < 5; ++i) {
        text_layer_destroy(s_hex_layers[i]);
    }
    text_layer_destroy(s_time_layer);

    for (int i = 0; i < 5; ++i) {
        free(s_buffers[i]);
        s_buffers[i] = 0;
    }
}

// initialize the main window and subscribe to services
// push the main window to the screen
static void init() {
    // allocate buffer to permanently display single rows
    for (int i = 0; i < 5; ++i){
        s_buffers[i] = calloc(15, sizeof(char));
    }

    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload,
    });

    window_stack_push(s_main_window, true);

    update_time(true);
    battery_callback(battery_state_service_peek());
    connection_callback(bluetooth_connection_service_peek());

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    connection_service_subscribe((ConnectionHandlers) {
            .pebble_app_connection_handler = connection_callback,
    });
}

// deinitialize the main window and free memory
static void deinit() {
    window_destroy(s_main_window);
}

// main function which initialize and deinitialize the watchface
int main(void) {
    init();
    app_event_loop();
    deinit();
}

