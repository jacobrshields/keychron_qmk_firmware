/*
 * LED_STATE_COLOR_FROM_BATTERY and LED_STATE_DEBOUNCE_INTERVAL_MS are defined in config.h.
 * 
 * LED_MATRIX_ENABLE and RGB_MATRIX_ENABLE are part of Quantum.
 */
#if (defined(LED_STATE_COLOR_FROM_BATTERY) || defined(LED_STATE_DEBOUNCE_INTERVAL_MS)) && (defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE))

#include "quantum.h"
#include "transport.h"
#include "usb_main.h"

// #define ENABLE_DEBUG_LEDS

/*
 * Enhancement: If the debounce technique proves insufficient, consider rejecting LED state changes
 *   that don't occur within X ticks/milliseconds of a relevant key press event. Or a combination,
 *   where LED state changes that occurr within X ticks/miliseconds of a relevant key press event
 *   are immediately accepted, but other LED state changes are accepted only after a relatively
 *   lengthy delay (e.g. hundreds of milliseconds, or a second).
 */

/*
 * To consider: Should we use the debounced LED state in LED_DRIVER_ALLOW_SHUTDOWN() in
 *   keyboards/keychron/common/wireless/indicator.c? The problem is that I don't think we can
 *   override it in user code--I think we would need to update the keyboard code. But I think that
 *   function is only checked if the current RGB matrix effect is RGB_MATRIX_NONE (see
 *   rgb_task_flush() in quantum/rgb_matrix/rgb_matrix.c), so it might not have a noticeable impact.
 */

#ifdef LED_MATRIX_ENABLE
    static __attribute__((unused)) void set_led_off(uint16_t led_index) {
        led_matrix_set_value(led_index, 0);
    }

    static __attribute__((unused)) void set_led_on(uint16_t led_index) {
        led_matrix_set_value(led_index, 255);
    }
#else // #ifdef RGB_MATRIX_ENABLE
    #include "utils/rgb.h"

    #ifdef LED_STATE_COLOR_FROM_BATTERY
        #include "utils/get_rgb_for_battery.h"

        static struct rgb led_rgb_color;
        static bool calculated_led_rgb_color = false;
    #else
        static struct rgb led_rgb_color = (struct rgb) { RGB_WHITE };
    #endif

    static __attribute__((unused)) void set_led_off(uint16_t led_index) {
        rgb_matrix_set_color(led_index, 0, 0, 0);
    }

    static __attribute__((unused)) void set_led_on(uint16_t led_index) {
        #ifdef LED_STATE_COLOR_FROM_BATTERY
            if (!calculated_led_rgb_color) {
                led_rgb_color = get_rgb_for_battery();
                calculated_led_rgb_color = true;
            }
        #endif
        rgb_matrix_set_color(led_index, led_rgb_color.r, led_rgb_color.g, led_rgb_color.b);
    }
#endif

#ifdef LED_STATE_DEBOUNCE_INTERVAL_MS
    #define DEFAULT_LED_STATE { num_lock: false, caps_lock: false, scroll_lock: false, compose: false, kana: false, reserved: 0 }

    static led_t debounced_led_state             = DEFAULT_LED_STATE;
    static led_t previous_led_state              = DEFAULT_LED_STATE;
    static fast_timer_t led_state_debounce_timer = 0;

    static void update_debounced_led_state(void) {
        led_t current_led_state = host_keyboard_led_state();

        // Always update the reserved bits immediately (no debouncing), because I don't know what
        // they're used for
        debounced_led_state.reserved = current_led_state.reserved;

        if (
               current_led_state.num_lock    != previous_led_state.num_lock
            || current_led_state.caps_lock   != previous_led_state.caps_lock
            || current_led_state.scroll_lock != previous_led_state.scroll_lock
            || current_led_state.kana        != previous_led_state.kana
            || current_led_state.caps_lock   != previous_led_state.caps_lock
        ) {
            led_state_debounce_timer = timer_read_fast() + LED_STATE_DEBOUNCE_INTERVAL_MS;
            // Guard against the rare case that we wrap around to exactly 0, because we use 0 to
            // indicate that the timer isn't running
            led_state_debounce_timer = MAX(1, led_state_debounce_timer);
            previous_led_state       = current_led_state;
        } else if (
               led_state_debounce_timer != 0
            && timer_expired_fast(timer_read_fast(), led_state_debounce_timer)
        ) {
            led_state_debounce_timer = 0;
            debounced_led_state      = current_led_state;
        }
    }
#endif // LED_STATE_DEBOUNCE_INTERVAL_MS

// Set LEDs based on the given LED state
static void set_leds(led_t led_state) {
    #if defined(LED_STATE_COLOR_FROM_BATTERY) && defined(RGB_MATRIX_ENABLE)
        calculated_led_rgb_color = false;
    #endif

    #if defined(NUM_LOCK_INDEX)
        if (led_state.num_lock) {
            set_led_on(NUM_LOCK_INDEX);
        }
    #endif

    #if defined(CAPS_LOCK_INDEX)
        if (led_state.caps_lock) {
            #if defined(DIM_CAPS_LOCK)
                set_led_off(CAPS_LOCK_INDEX);
            #else
                set_led_on(CAPS_LOCK_INDEX);
            #endif
        }
    #endif

    #if defined(SCROLL_LOCK_INDEX)
        if (led_state.scroll_lock) {
            set_led_on(SCROLL_LOCK_INDEX);
        }
    #endif

    #if defined(COMPOSE_LOCK_INDEX)
        if (led_state.compose) {
            set_led_on(COMPOSE_LOCK_INDEX);
        }
    #endif

    #if defined(KANA_LOCK_INDEX)
        if (led_state.kana) {
            set_led_on(KANA_LOCK_INDEX);
        }
    #endif
}

#ifdef ENABLE_DEBUG_LEDS
    #ifdef LED_MATRIX_ENABLE
        #define SET_DEBUG_LED_ON(led_index) led_matrix_set_value(led_index, 255)
    #else // #ifdef RGB_MATRIX_ENABLE
        #define SET_DEBUG_LED_ON(led_index) rgb_matrix_set_color(led_index, 255, 255, 255)
    #endif

    // Use LEDs of number keys 1-8 to display the raw bits of the current (non-debounced) LED state
    static void set_debug_leds(void) {
        uint8_t raw_led_state = host_keyboard_led_state().raw;
        if (raw_led_state & (1<<7)) { // reserved bit
            SET_DEBUG_LED_ON(17); // 1 key
        }
        if (raw_led_state & (1<<6)) { // reserved bit
            SET_DEBUG_LED_ON(18); // 2 key
        }
        if (raw_led_state & (1<<5)) { // reserved bit
            SET_DEBUG_LED_ON(19); // 3 key
        }
        if (raw_led_state & (1<<4)) { // kana bit
            SET_DEBUG_LED_ON(20); // 4 key
        }
        if (raw_led_state & (1<<3)) { // compose bit
            SET_DEBUG_LED_ON(21); // 5 key
        }
        if (raw_led_state & (1<<2)) { // scroll_lock bit
            SET_DEBUG_LED_ON(22); // 6 key
        }
        if (raw_led_state & (1<<1)) { // caps_lock bit
            SET_DEBUG_LED_ON(23); // 7 key
        }
        if (raw_led_state & (1<<0)) { // num_lock bit
            SET_DEBUG_LED_ON(24); // 8 key
        }
    }
#endif

// Overrides weak os_state_indicate method from keyboards/keychron/common/wireless/indicator.c
void os_state_indicate(void) {
    // Copied from indicator.c
    #if defined(RGB_DISABLE_WHEN_USB_SUSPENDED) || defined(LED_DISABLE_WHEN_USB_SUSPENDED)
        if (get_transport() == TRANSPORT_USB && USB_DRIVER.state == USB_SUSPENDED) return;
    #endif

    #ifdef LED_STATE_DEBOUNCE_INTERVAL_MS
        update_debounced_led_state();
        set_leds(debounced_led_state);
    #else
        set_leds(host_keyboard_led_state());
    #endif

    #ifdef ENABLE_DEBUG_LEDS
        set_debug_leds();
    #endif
}

#endif // (LED_STATE_COLOR_FROM_BATTERY || LED_STATE_DEBOUNCE_INTERVAL_MS) && (LED_MATRIX_ENABLE || RGB_MATRIX_ENABLE)
