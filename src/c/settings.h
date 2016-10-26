#pragma once
#include <pebble.h>

// persistent storage key
#define SETTINGS_KEY    1

// define our settings struct
typedef struct ClaySettings {
    GColor BackgroundColor;
    GColor AccentColor;
    GColor MainColor;
    bool VibeOnDisconnect;
} ClaySettings;

typedef void(* SettingsChangedCallback)(ClaySettings *settings);

ClaySettings* settings_load();
void settings_register_settings_changed(SettingsChangedCallback callback);
