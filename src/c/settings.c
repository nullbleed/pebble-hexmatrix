#include "settings.h"

#define INBOX_SIZE 128
#define OUTBOX_SIZE 128

static ClaySettings s_settings;
static SettingsChangedCallback s_settings_changed = NULL;

static void settings_save();

static void settings_inbox_received_handler(DictionaryIterator *iter, void *context) {
    // read color preferences
    Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
    if(bg_color_t) {
        s_settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
    }

    Tuple *ac_color_t = dict_find(iter, MESSAGE_KEY_AccentColor);
    if(ac_color_t) {
        s_settings.AccentColor = GColorFromHEX(ac_color_t->value->int32);
    }    

    Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_MainColor);
    if(fg_color_t) {
        s_settings.MainColor = GColorFromHEX(fg_color_t->value->int32);
    }

    Tuple *vibe_on_disconnect_t = dict_find(iter, MESSAGE_KEY_VibrateOnDisconnect);
    if (vibe_on_disconnect_t) {
        s_settings.VibeOnDisconnect = vibe_on_disconnect_t->value->int32 == 1;
    }

    // save new settings
    settings_save();

    // notify callback
    if (s_settings_changed != NULL) {
        s_settings_changed(&s_settings);
    }
}

// initialize the default settings
static void settings_init_default() {
    s_settings.BackgroundColor = GColorBlack;
    s_settings.AccentColor = GColorCyan;
    s_settings.MainColor = GColorWhite;
    s_settings.VibeOnDisconnect = false;
}

// save the settings to persistent storage
static void settings_save() {
    // write new setttings to persistent storage
    persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

// read settings from persistent storage
ClaySettings* settings_load() {
    // load the default settings
    settings_init_default();

    // Open AppMessage connection
    app_message_register_inbox_received(settings_inbox_received_handler);
    app_message_open(INBOX_SIZE, OUTBOX_SIZE);

    // read settings from persistent storage, if they exist
    persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
    return &s_settings;
}

void settings_register_settings_changed(SettingsChangedCallback callback) {
    s_settings_changed = callback;
}
