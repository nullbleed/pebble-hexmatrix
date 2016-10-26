#include <pebble.h>
#include "watchface.h"
#include "settings.h"

static bool s_vibe_on_disconnect;

// (dis-)connect vibe patterns
static const uint32_t const short_vibe[] = { 80 };
static const uint32_t const double_vibe[] = { 50, 100, 80 };

// callback for the time service
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    main_window_update_time(false);
}

// update battery level
static void battery_callback(BatteryChargeState state) {
    main_window_update_battery(state.charge_percent);
}

// update bt connection status
static void connection_callback(bool connected) {
    main_window_update_connection(connected);

    if (connected) {
        if (s_vibe_on_disconnect == true) {
            vibes_enqueue_custom_pattern((VibePattern) {
                    .durations = short_vibe,
                    .num_segments = ARRAY_LENGTH(short_vibe),
            });
        }
    } else {
        if (s_vibe_on_disconnect == true) {
            vibes_enqueue_custom_pattern((VibePattern) {
                    .durations = double_vibe,
                    .num_segments = ARRAY_LENGTH(double_vibe),
            });
        }
    }
}

// update attributes according to changed settings
static void settings_changed_handler(ClaySettings *settings) {
    // update time variables
    s_vibe_on_disconnect = settings->VibeOnDisconnect;

    main_window_update_settings(settings);
}

// initialize the main window and subscribe to services
// push the main window to the screen
static void init() {
    // create main window
    main_window_push();

    // load settings
    ClaySettings *settings = settings_load();

    main_window_update_settings(settings);
    s_vibe_on_disconnect = settings->VibeOnDisconnect;

    // update time and status data
    main_window_update_time(true);

    BatteryChargeState state = battery_state_service_peek();
    bool connected = bluetooth_connection_service_peek();

    main_window_update_battery(state.charge_percent);
    main_window_update_connection(connected);

    // subscribe to events
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    connection_service_subscribe((ConnectionHandlers) {
            .pebble_app_connection_handler = connection_callback,
    });

    settings_register_settings_changed(settings_changed_handler);
}

// deinitialize the main window and free memory
static void deinit() {
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    connection_service_unsubscribe();

    main_window_deinit();
}

// main function which initialize and deinitialize the watchface
int main(void) {
    init();
    app_event_loop();
    deinit();
}

