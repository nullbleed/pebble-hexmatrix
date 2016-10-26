#pragma once
#include <pebble.h>
#include "settings.h"

#define TEXT_LAYER_DATE     0
#define TEXT_LAYER_WEEKDAY  1
#define TEXT_LAYER_TIME     2
#define TEXT_LAYER_RFFU     3
#define TEXT_LAYER_STAT     4

typedef struct Date {
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
    int weekday;
} Date;

typedef struct State {
    int connection;
    int battery;
} State;

typedef struct MainWindowData {
    GFont font;
    GFont font_bold;
    struct {
        GColor bg;
        GColor main;
        GColor accent;
    } colors;
    struct {
        TextLayer *hex_layers[5];
        TextLayer *time_layer;
    } layers;
    struct {
        char *hex_buffers[5];
        char *time_buffer;
    } buffers;
    Date date;
    State state;
} MainWindowData;

void main_window_push();
void main_window_deinit();

void main_window_update_time(bool init);
void main_window_update_connection(bool connected);
void main_window_update_battery(int percent);
void main_window_update_settings(ClaySettings* settings);
