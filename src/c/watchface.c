#include "watchface.h"

static Window *s_main_window;

static int s_randoms[16];

// render selected text layer
static void render_row(int row) {
    MainWindowData *data = window_get_user_data(s_main_window);
    Date *date = &(data->date);
    State *state = &(data->state);

    if (row == TEXT_LAYER_DATE) {
        snprintf(data->buffers.hex_buffers[row], 15, "%01X %02X %02X %02X %01X", s_randoms[0] % 16, date->day, date->month, date->year, s_randoms[1] % 16);
    } else if (row == TEXT_LAYER_WEEKDAY) {
        snprintf(data->buffers.hex_buffers[row], 15, "%01X %02X %02X %02X %01X", s_randoms[2] % 16, s_randoms[3], date->weekday, s_randoms[4], s_randoms[5] % 16);
    } else if (row == TEXT_LAYER_TIME) {
        snprintf(data->buffers.hex_buffers[row], 15, "%01X          %01X", s_randoms[6] % 16, s_randoms[7] % 16);
    } else if (row == TEXT_LAYER_RFFU) {
        snprintf(data->buffers.hex_buffers[row], 15, "%01X %02X %02X %02X %01X", s_randoms[8] % 16, s_randoms[9], s_randoms[10], s_randoms[11], s_randoms[12] % 16);
    } else if (row == TEXT_LAYER_STAT) {
        snprintf(data->buffers.hex_buffers[row], 15, "%01X %02X %02X %02X %01X", s_randoms[13] % 16, state->connection, s_randoms[14], state->battery, s_randoms[15] % 16);
    }

    if (row >= 0 && row < 6) {
        text_layer_set_text(data->layers.hex_layers[row], data->buffers.hex_buffers[row]);
    }
}

static void render_time() {
    MainWindowData *data = window_get_user_data(s_main_window);
    Date *date = &(data->date);

    snprintf(data->buffers.time_buffer, 15, "   %02X %02X %02X   ", date->hour, date->minute, date->second);
    text_layer_set_text(data->layers.time_layer, data->buffers.time_buffer);
}

// initialize the different layers when the window is created
static void main_window_load(Window *window) {
    // get root layer and boundaries of the watch
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    MainWindowData *data = window_get_user_data(window);

    // set the background color for the root window
    window_set_background_color(s_main_window, data->colors.bg);

    // get font
    data->font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINESS_24));
    data->font_bold = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TERMINESS_BOLD_24));

    // create text layers
    for (int i = 0; i < 5; ++i) {
        TextLayer *layer = data->layers.hex_layers[i] = text_layer_create(GRect(0, 4 + (i * 32), bounds.size.w, 24));

        // set options for text layer
        text_layer_set_background_color(layer, GColorClear);
        text_layer_set_text_color(layer, data->colors.accent);
        text_layer_set_font(layer, data->font);
        text_layer_set_text_alignment(layer, GTextAlignmentCenter);
    }

    // create time text layer
    //TODO: fix this weird offsets caused by the bigger font size
    TextLayer *time_layer = data->layers.time_layer = text_layer_create(GRect(-26, 68, bounds.size.w + 50, 24));

    // set options for the time text layer
    text_layer_set_background_color(time_layer, GColorClear);
    text_layer_set_text_color(time_layer, data->colors.main);
    text_layer_set_font(time_layer, data->font_bold);
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    
    // add the layers to the main window
    for (int i = 0; i < 5; ++i) {
        layer_add_child(window_layer, text_layer_get_layer(data->layers.hex_layers[i]));
    }
    layer_add_child(window_layer, text_layer_get_layer(data->layers.time_layer));
}

// free layers on destroy
static void main_window_unload(Window *window) {
    MainWindowData *data = window_get_user_data(window);

    for (int i = 0; i < 5; ++i) {
        text_layer_destroy(data->layers.hex_layers[i]);
        free(data->buffers.hex_buffers[i]);
    }
    text_layer_destroy(data->layers.time_layer);
    free(data->buffers.time_buffer);
}

void main_window_push() {
    if (!s_main_window) {
        // create main window
        MainWindowData *data = malloc(sizeof(MainWindowData));
        memset(data, 0, sizeof(MainWindowData));

        for (int i = 0; i < 5; ++i) {
            char *buf = data->buffers.hex_buffers[i] = calloc(16, sizeof(char));
            memset(buf, 0, 16 * sizeof(char));
        }
        char *time_buf = data->buffers.time_buffer = calloc(16, sizeof(char));
        memset(time_buf, 0, 16 * sizeof(char));

        s_main_window = window_create();
        window_set_user_data(s_main_window, data);

        window_set_window_handlers(s_main_window, (WindowHandlers) {
                .load = main_window_load,
                .unload = main_window_unload,
        });
    }

    window_stack_push(s_main_window, true);
}

void main_window_deinit() {
    if (s_main_window) {
        window_destroy(s_main_window);
        s_main_window = NULL;
    }
}

// update time
void main_window_update_time(bool init) {
    MainWindowData *data = window_get_user_data(s_main_window);
    Date *date = &(data->date);

    // get time
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // get seconds as int
    static char s_second_buffer[8];
    strftime(s_second_buffer, sizeof(s_second_buffer), "%S", tick_time);
    date->second = atoi(s_second_buffer);

    if (init) {
        srand(temp);
    }
    
    // generate randoms
    if (date->second == 0 || init) {
        for (int i = 0; i < 16; ++i) {
            s_randoms[i] = rand() % 256;
        } 
    }

    if (date->second == 0 || init) { // EOM
        // get minutes as int
        static char s_minute_buffer[8];
        strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
        date->minute = atoi(s_minute_buffer);

        if (date->minute == 0 || init) { // EOH
            // get hours as int
            static char s_hour_buffer[8];
            strftime(s_hour_buffer, sizeof(s_hour_buffer), "%H", tick_time);
            date->hour = atoi(s_hour_buffer);

            if (date->hour == 0 || init) { // EOD
                // get day as int
                static char s_day_buffer[8];
                strftime(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time);
                date->day = atoi(s_day_buffer);

                // get weekday as int
                static char s_weekday_buffer[8];
                strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%w", tick_time);
                date->weekday = ((atoi(s_weekday_buffer) + 6) % 7) + 1;

                // get month as int
                static char s_month_buffer[8];
                strftime(s_month_buffer, sizeof(s_month_buffer), "%m", tick_time);
                date->month = atoi(s_month_buffer);

                // get year as int
                static char s_year_buffer[8];
                strftime(s_year_buffer, sizeof(s_year_buffer), "%y", tick_time);
                date->year = atoi(s_year_buffer);

                // display updated date
                render_row(TEXT_LAYER_DATE);
                render_row(TEXT_LAYER_WEEKDAY);
            }
        }
    }

    // display updated time
    render_time();

    if (date->second == 0 || init) {
        if (date->hour != 0) {
            render_row(TEXT_LAYER_DATE);
            render_row(TEXT_LAYER_WEEKDAY);
        }
        render_row(TEXT_LAYER_TIME);
        render_row(TEXT_LAYER_RFFU);
        render_row(TEXT_LAYER_STAT);
    }
}

void main_window_update_connection(bool connected) {
    MainWindowData *data = window_get_user_data(s_main_window);
    data->state.connection = connected ? 255 : 0;
}

void main_window_update_battery(int percent) {
    MainWindowData *data = window_get_user_data(s_main_window);
    data->state.battery = percent * 255 / 100;
}

void main_window_update_settings(ClaySettings* settings) {
    if (s_main_window) {
        MainWindowData *data = window_get_user_data(s_main_window);

        data->colors.bg = settings->BackgroundColor;
        data->colors.accent = settings->AccentColor;
        data->colors.main = settings->MainColor;

        // set main window color
        window_set_background_color(s_main_window, data->colors.bg);

        // update hex layers
        for (int i = 0; i < 5; ++i) {
            text_layer_set_text_color(data->layers.hex_layers[i], data->colors.accent);
        }

        // update time layer
        text_layer_set_text_color(data->layers.time_layer, data->colors.main);
    }
}
