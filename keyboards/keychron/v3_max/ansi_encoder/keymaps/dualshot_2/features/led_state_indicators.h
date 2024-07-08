#pragma once

#if defined(LED_STATE_RESIST_UNEXPECTED_CHANGES) && (defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE))

#include "quantum.h"

bool process_record_led_state_indicators(uint16_t keycode, keyrecord_t *record);

#endif // LED_STATE_RESIST_UNEXPECTED_CHANGES && (LED_MATRIX_ENABLE || RGB_MATRIX_ENABLE)
