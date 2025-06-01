/*
 * WINDOWS_KEY_LOCK is defined in config.h.
 */
#if defined(WINDOWS_KEY_LOCK)

#include "quantum.h"

#include "custom_keycodes.h"

static bool windows_key_locked = false;

static void toggle_windows_key_lock(void) {
    windows_key_locked = !windows_key_locked;
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
                return !windows_key_locked; // Stop processing if and only if Windows key lock is enabled
            }
            return true; // Continue processing
        default:
            return true; // Continue processing
    }
}

#endif // WINDOWS_KEY_LOCK
