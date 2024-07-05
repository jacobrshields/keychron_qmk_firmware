// Reference: https://docs.qmk.fm/features/rgb_matrix#custom-rgb-matrix-effects
// !!! DO NOT ADD #pragma once !!! //

/*
 * ENABLE_RGB_MATRIX_CUSTOM_REACTIVE_ESCAPE_KEY is defined indirectly via the
 * .rgb_matrix.animations.custom_reactive_escape_key entry in info.json. It gets included into
 * .build/obj_keychron_v3_max_ansi_encoder_dualshot_2/src/info_config.h via
 * the lib/python/qmk/cli/generate/config_h.py script.
 * 
 * RGB_MATRIX_KEYREACTIVE_ENABLED is part of Quantum and is used by other reactive animations.
 */
#if defined(ENABLE_RGB_MATRIX_CUSTOM_REACTIVE_ESCAPE_KEY) && defined(RGB_MATRIX_KEYREACTIVE_ENABLED)

// Declare custom effect if it's enabled
RGB_MATRIX_EFFECT(REACTIVE_ESCAPE_KEY)

// Declare implementation only when RGB_MATRIX_CUSTOM_EFFECT_IMPLS is defined
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#ifndef ESCAPE_KEY_INDEX
    #define ESCAPE_KEY_INDEX 0
#endif

static uint16_t get_tick_of_last_hit(uint8_t led_index) {
    // Reverse search to find most recent key hit
    for (int8_t hit_index = g_last_hit_tracker.count - 1; hit_index >= 0; hit_index--) {
        if (g_last_hit_tracker.index[hit_index] == led_index) {
            return g_last_hit_tracker.tick[hit_index];
        }
    }
    return -1;
}

/*
 * hsv: Configured HSV
 * animation_offset: Amount of animation completed so far, normalized to range [0, 255]
 */
static HSV get_animated_hsv(HSV hsv, uint8_t animation_offset) {
    // Copied from other reactive animations
    #ifdef RGB_MATRIX_SOLID_REACTIVE_GRADIENT_MODE
        hsv.h = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed, 8) >> 4);
    #endif
    hsv.v = scale8(hsv.v, 255 - animation_offset); // Fade out over time
    return hsv;
}

static RGB calculate_rgb(uint8_t led) {
    uint16_t tick_of_last_hit = get_tick_of_last_hit(led); // -1 if no recent hits

    if (tick_of_last_hit < 0) {
        return (RGB) { RGB_OFF };
    }

    // Animation duration in ticks. Range: [257, 65,535]
    uint16_t animation_duration = 65535 / qadd8(rgb_matrix_config.speed, 1);

    // Number of ticks since the start of the animation. Stop counting ticks after the end
    // of the animation. Range: [0, 65,535]
    uint16_t animation_tick = MIN(tick_of_last_hit, animation_duration);

    // Amount of animation completed so far, normalized to range [0, 255].
    // Even without the cast to uint8_t, the max value is 255 because the max is:
    //     max(animation_tick)  * (speed + 1) / 256
    //   = animation_duration   * (speed + 1) / 256
    //   = 65,535 / (speed + 1) * (speed + 1) / 256
    //   = 65,535 / 256
    //   = 255
    uint8_t animation_offset = (uint8_t) scale16by8(animation_tick, qadd8(rgb_matrix_config.speed, 1));

    HSV hsv = get_animated_hsv(rgb_matrix_config.hsv, animation_offset);
    return rgb_matrix_hsv_to_rgb(hsv);
}

static void REACTIVE_ESCAPE_KEY_init(effect_params_t* params) {
    // No-op
}

static bool REACTIVE_ESCAPE_KEY_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    for (uint8_t led_index = led_min; led_index < led_max; led_index++) {
        // This is the RGB_MATRIX_TEST_LED_FLAGS() macro, expanded for clarity.
        // I don't understand what this test is for, but it's in the reference.
        if (!HAS_ANY_FLAGS(g_led_config.flags[led_index], params->flags)) {
            continue;
        }

        if (led_index == ESCAPE_KEY_INDEX) {
            RGB rgb = calculate_rgb(led_index);
            rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(led_index, RGB_OFF);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

static bool REACTIVE_ESCAPE_KEY(effect_params_t* params) {
    if (params->init) {
        REACTIVE_ESCAPE_KEY_init(params);
        return false; // Stop processing?
    } else {
        return REACTIVE_ESCAPE_KEY_run(params);
    }
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ENABLE_RGB_MATRIX_CUSTOM_REACTIVE_ESCAPE_KEY && RGB_MATRIX_KEYREACTIVE_ENABLED
