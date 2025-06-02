#pragma once

/*
 * WINDOWS_KEY_LOCK is defined in config.h.
 */
#if defined(WINDOWS_KEY_LOCK)

#include "quantum.h"

bool process_record_windows_key_lock(uint16_t keycode, keyrecord_t *record);

/*
 * RGB_MATRIX_ENABLE is part of Quantum.
 */
#if defined(RGB_MATRIX_ENABLE)

bool rgb_matrix_indicators_windows_key_lock(void);

#endif // RGB_MATRIX_ENABLE

#endif // WINDOWS_KEY_LOCK
