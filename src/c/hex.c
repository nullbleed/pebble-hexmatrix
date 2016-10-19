#include <pebble.h>


static Window *s_main_window;
static TextLayer *s_first_row_layer;
static TextLayer *s_second_row_layer;
static TextLayer *s_third_row_layer;
static TextLayer *s_third_left_row_layer;
static TextLayer *s_third_right_row_layer;
static TextLayer *s_fourth_row_layer;
static TextLayer *s_fifth_row_layer;

static GFont s_time_font;
static int s_battery_level;

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
}

static void update_time() {
    static int randoms[16];
    static int initialized = 0;

    // get time
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    // get hours as int
    static char s_hour_buffer[8];
    static int hours;
    strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", tick_time);
    hours = atoi(s_hour_buffer);

    // get minutes as int
    static char s_minute_buffer[8];
    static int minutes;
    strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
    minutes = atoi(s_minute_buffer);

    // get seconds as int
    static char s_second_buffer[8];
    static int seconds;
    strftime(s_second_buffer, sizeof(s_second_buffer), "%S", tick_time);
    seconds = atoi(s_second_buffer);

    // get year as int
    static char s_year_buffer[8];
    static int years;
    strftime(s_year_buffer, sizeof(s_year_buffer), "%y", tick_time);
    years = atoi(s_year_buffer);

    // get month as int
    static char s_month_buffer[8];
    static int months;
    strftime(s_month_buffer, sizeof(s_month_buffer), "%m", tick_time);
    months = atoi(s_month_buffer);

    // get day as int
    static char s_day_buffer[8];
    static int days;
    strftime(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time);
    days = atoi(s_day_buffer);

    // get weekday as int
    static char s_weekday_buffer[8];
    static int weekdays;
    strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%w", tick_time);
    weekdays = atoi(s_weekday_buffer);
    weekdays = ((weekdays + 7 - 1) % 7) + 1;

    // generate randoms
    if (seconds == 0 || initialized == 0) {
        for (int i=0; i < 16; i++) {
            randoms[i] = rand() % 255;
            initialized = 1;
        } 
        
        // contruct date row
        static char s_first_row_buffer[15];
        snprintf(s_first_row_buffer, 15, "%02X %02X %02X %02X %02X", randoms[0], days, months, years, randoms[1]);
        text_layer_set_text(s_first_row_layer, s_first_row_buffer);

        // contruct weekday row
        static char s_second_row_buffer[15];
        snprintf(s_second_row_buffer, 15, "%02X %02X %02X %02X %02X", randoms[2], randoms[3], weekdays, randoms[4], randoms[5]);
        text_layer_set_text(s_second_row_layer, s_second_row_buffer);

        // construct time row randoms
        static char s_third_left_row_buffer[9];
        snprintf(s_third_left_row_buffer, 9, "%02X", randoms[6]);
        text_layer_set_text(s_third_left_row_layer, s_third_left_row_buffer);
        
        static char s_third_right_row_buffer[9];
        snprintf(s_third_right_row_buffer, 9, "%02X", randoms[7]);
        text_layer_set_text(s_third_right_row_layer, s_third_right_row_buffer);

        // contruct weekday row
        static char s_fourth_row_buffer[15];
        snprintf(s_fourth_row_buffer, 15, "%02X %02X %02X %02X %02X", randoms[8], randoms[9], randoms[10], randoms[11], randoms[12]);
        text_layer_set_text(s_fourth_row_layer, s_fourth_row_buffer);

        // contruct info row
        static char s_fifth_row_buffer[15];
        snprintf(s_fifth_row_buffer, 15, "%02X %02X %02X %02X %02X", randoms[13], s_battery_level, randoms[14], s_battery_level, randoms[15]);
        text_layer_set_text(s_fifth_row_layer, s_fifth_row_buffer);
    }

    // contruct time row
    static char s_third_row_buffer[9];
    snprintf(s_third_row_buffer, 9, "%02X %02X %02X", hours, minutes, seconds);
    text_layer_set_text(s_third_row_layer, s_third_row_buffer);
}


// callback for the time service which marks the time layer as dirty to refresh it
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    battery_callback(battery_state_service_peek());
    update_time();
}

// initialize the different layers when the window is created
static void main_window_load(Window *window) {
    // colors
#if defined(PBL_COLOR)
    GColor main_color = GColorWhite;
    GColor accent_color = GColorCyan;
    GColor background_color = GColorClear;
#else
    GColor main_color = GColorWhite;
    GColor accent_color = GColorLightGray;
    GColor background_color = GColorClear;
#endif

    // get root layer and boundaries of the watch
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // set the background color for the root window
    window_set_background_color(s_main_window, GColorBlack);

    // get font
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINESS_20));

    // create first layer
    s_first_row_layer = text_layer_create(GRect(0, 8, bounds.size.w, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_first_row_layer, background_color);
    text_layer_set_text_color(s_first_row_layer, accent_color);
    text_layer_set_text(s_first_row_layer, "00 13 0A 10 00");
    text_layer_set_font(s_first_row_layer, s_time_font);
    text_layer_set_text_alignment(s_first_row_layer, GTextAlignmentCenter);

    // create second layer
    s_second_row_layer = text_layer_create(GRect(0, 40, bounds.size.w, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_second_row_layer, background_color);
    text_layer_set_text_color(s_second_row_layer, accent_color);
    text_layer_set_text(s_second_row_layer, "00 13 0A 10 00");
    text_layer_set_font(s_second_row_layer, s_time_font);
    text_layer_set_text_alignment(s_second_row_layer, GTextAlignmentCenter);

    // create third layer
    s_third_row_layer = text_layer_create(GRect(20, 72, bounds.size.w - 40, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_third_row_layer, background_color);
    text_layer_set_text_color(s_third_row_layer, main_color);
    text_layer_set_text(s_third_row_layer, "13 0A 10");
    text_layer_set_font(s_third_row_layer, s_time_font);
    text_layer_set_text_alignment(s_third_row_layer, GTextAlignmentCenter);
    
    // create third left layer
    s_third_left_row_layer = text_layer_create(GRect(2, 72, 20, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_third_left_row_layer, background_color);
    text_layer_set_text_color(s_third_left_row_layer, accent_color);
    text_layer_set_text(s_third_left_row_layer, "00");
    text_layer_set_font(s_third_left_row_layer, s_time_font);
    text_layer_set_text_alignment(s_third_left_row_layer, GTextAlignmentCenter);
    
    // create third right layer
    s_third_right_row_layer = text_layer_create(GRect(122, 72, 20, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_third_right_row_layer, background_color);
    text_layer_set_text_color(s_third_right_row_layer, accent_color);
    text_layer_set_text(s_third_right_row_layer, "00");
    text_layer_set_font(s_third_right_row_layer, s_time_font);
    text_layer_set_text_alignment(s_third_right_row_layer, GTextAlignmentCenter);

    // create fourth layer
    s_fourth_row_layer = text_layer_create(GRect(0, 104, bounds.size.w, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_fourth_row_layer, background_color);
    text_layer_set_text_color(s_fourth_row_layer, accent_color);
    text_layer_set_text(s_fourth_row_layer, "00 13 0A 10 00");
    text_layer_set_font(s_fourth_row_layer, s_time_font);
    text_layer_set_text_alignment(s_fourth_row_layer, GTextAlignmentCenter);

    // create fifth layer
    s_fifth_row_layer = text_layer_create(GRect(0, 136, bounds.size.w, 20));

    // set options for the date text layer
    text_layer_set_background_color(s_fifth_row_layer, background_color);
    text_layer_set_text_color(s_fifth_row_layer, accent_color);
    text_layer_set_text(s_fifth_row_layer, "00 13 0A 10 00");
    text_layer_set_font(s_fifth_row_layer, s_time_font);
    text_layer_set_text_alignment(s_fifth_row_layer, GTextAlignmentCenter);

    // add the layers to the main window
    layer_add_child(window_layer, text_layer_get_layer(s_first_row_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_second_row_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_third_row_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_third_left_row_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_third_right_row_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_fourth_row_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_fifth_row_layer));
}

// free layers on destroy
static void main_window_unload(Window *window) {
    text_layer_destroy(s_first_row_layer);
    text_layer_destroy(s_second_row_layer);
    text_layer_destroy(s_third_row_layer);
    text_layer_destroy(s_third_left_row_layer);
    text_layer_destroy(s_third_right_row_layer);
    text_layer_destroy(s_fourth_row_layer);
    text_layer_destroy(s_fifth_row_layer);
}

// initialize the main window and subscribe to services
// push the main window to the screen
static void init() {
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload
            });

    window_stack_push(s_main_window, true);

    update_time();

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
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

