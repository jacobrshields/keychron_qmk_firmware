/*
 * WINDOWS_KEY_LOCK is defined in config.h.
 */
#if defined(WINDOWS_KEY_LOCK)

#include "quantum.h"

#include "custom_keycodes.h"
#include "utils/timers.h"

#ifndef LEFT_WINKEY_INDEX
    #define LEFT_WINKEY_INDEX 77
#endif
#ifndef RIGHT_WINKEY_INDEX
    #define RIGHT_WINKEY_INDEX 81
#endif

#define INDICATOR_DURATION_MS 500

static bool windows_key_locked = false;
static fast_timer_t indicator_timer = TIMER_STOPPED;

static void toggle_windows_key_lock(void) {
    windows_key_locked = !windows_key_locked;

    // Show indicator to inform user whether Windows key is now locked or unlocked
    indicator_timer = START_FAST_TIMER(INDICATOR_DURATION_MS);
}

bool process_record_windows_key_lock(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case WIN_LOCK:
            if (record->event.pressed) {
                toggle_windows_key_lock();
            }
            return false; // Stop processing
        case KC_LWIN:
        case KC_RWIN:
            if (record->event.pressed) {
                if (windows_key_locked) {
                    // Show indicator to inform user the Windows key is currently locked
                    indicator_timer = START_FAST_TIMER(INDICATOR_DURATION_MS);
                    return false; // Stop processing
                }
                
                // Clear indicator for the case where user presses the Windows key immediately after unlocking (because
                // the user no longer needs to be informed the key is unlocked: they just activated it)
                indicator_timer = TIMER_STOPPED;
            }
            return true; // Continue processing
        default:
            return true; // Continue processing
    }
}

/*
 * RGB_MATRIX_ENABLE is part of Quantum.
 */
#if defined(RGB_MATRIX_ENABLE)

bool rgb_matrix_indicators_windows_key_lock(void) {
    if (indicator_timer != TIMER_STOPPED) {
        if (timer_expired_fast(timer_read_fast(), indicator_timer)) {
            indicator_timer = TIMER_STOPPED;
        } else {
            // Using the same RGB values from utils/get_rgb_for_battery.c
            if (windows_key_locked) {
                rgb_matrix_set_color(LEFT_WINKEY_INDEX,  255, 4, 4);
                rgb_matrix_set_color(RIGHT_WINKEY_INDEX, 255, 4, 4);
            } else {
                rgb_matrix_set_color(LEFT_WINKEY_INDEX,  96, 255, 0);
                rgb_matrix_set_color(RIGHT_WINKEY_INDEX, 96, 255, 0);
            }
        }
    }
    return true; // Continue processing
}

#endif // RGB_MATRIX_ENABLE

#endif // WINDOWS_KEY_LOCK
