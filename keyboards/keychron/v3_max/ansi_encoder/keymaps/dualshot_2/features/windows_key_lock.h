#pragma once

/*
 * WINDOWS_KEY_LOCK is defined in config.h.
 */
#if defined(WINDOWS_KEY_LOCK)

#include "quantum.h"

bool process_record_windows_key_lock(uint16_t keycode, keyrecord_t *record);

#endif // WINDOWS_KEY_LOCK
