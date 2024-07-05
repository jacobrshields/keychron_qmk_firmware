#pragma once

/*
 * ENABLE_TOGGLEABLE_BATTERY_INDICATOR is defined in config.h.
 * 
 * RGB_MATRIX_ENABLE is part of Quantum.
 */
#if defined(ENABLE_TOGGLEABLE_BATTERY_INDICATOR) && defined(RGB_MATRIX_ENABLE)

bool process_record_toggleable_battery_indicator(uint16_t keycode, keyrecord_t *record);

bool rgb_matrix_indicators_toggleable_battery_indicator(void);

#endif // ENABLE_TOGGLEABLE_BATTERY_INDICATOR && RGB_MATRIX_ENABLE
