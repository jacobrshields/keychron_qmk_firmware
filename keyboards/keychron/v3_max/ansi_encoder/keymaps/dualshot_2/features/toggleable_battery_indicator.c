/*
 * ENABLE_TOGGLEABLE_BATTERY_INDICATOR is defined in config.h.
 * 
 * RGB_MATRIX_ENABLE is part of Quantum.
 */
#if defined(ENABLE_TOGGLEABLE_BATTERY_INDICATOR) && defined(RGB_MATRIX_ENABLE)

#include "quantum.h"
#include "indicator.h"

#include "custom_keycodes.h"
#include "utils/get_rgb_for_battery.h"

#ifndef BATTERY_INDICATOR_INDEX
    #define BATTERY_INDICATOR_INDEX 14
#endif

/*
 * Enhancement: Allow customizing the default value of this variable. I guess we should call
  *   indicator_enable() immediately if this is initialized to true.
 */
static bool battery_indicator_enabled = false;

static void toggle_battery_indicator(void) {
    battery_indicator_enabled = !battery_indicator_enabled;

    if (battery_indicator_enabled) {
        indicator_enable();
    }
}

bool process_record_toggleable_battery_indicator(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case BAT_TOG:
            if (record->event.pressed) {
                toggle_battery_indicator();
            }
            return false; // Stop processing
        default:
            return true; // Continue processing
    }
}

bool rgb_matrix_indicators_toggleable_battery_indicator(void) {
    if (battery_indicator_enabled) {
        struct rgb rgb = get_rgb_for_battery();
        rgb_matrix_set_color(BATTERY_INDICATOR_INDEX, rgb.r, rgb.g, rgb.b);
    }
    return true; // Continue processing
}

#endif // ENABLE_TOGGLEABLE_BATTERY_INDICATOR && RGB_MATRIX_ENABLE
