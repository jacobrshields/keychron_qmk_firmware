#pragma once

#include "keychron_common.h"

enum custom_keycodes {
    // NEW_SAFE_RANGE starts after Keychron's custom keycodes (defined in
    //   keyboards/keychron/common/keychron_common.h)
    _CUSTOM_KEYCODES_INITIALIZER = NEW_SAFE_RANGE - 1,
    #if defined(ENABLE_TOGGLEABLE_BATTERY_INDICATOR) && defined(RGB_MATRIX_ENABLE)
        BAT_TOG,
    #endif
    NEW_NEW_SAFE_RANGE,
};
