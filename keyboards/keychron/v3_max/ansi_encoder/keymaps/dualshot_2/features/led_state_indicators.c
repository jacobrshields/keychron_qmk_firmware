/*
 * LED_STATE_COLOR_FROM_BATTERY and LED_STATE_RESIST_UNEXPECTED_CHANGES are defined in config.h.
 * 
 * LED_MATRIX_ENABLE and RGB_MATRIX_ENABLE are part of Quantum.
 */
#if (defined(LED_STATE_COLOR_FROM_BATTERY) || defined(LED_STATE_RESIST_UNEXPECTED_CHANGES)) && (defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE))

#include "quantum.h"
#include "transport.h"
#include "usb_main.h"

#include "led_state_indicators.h"

// #define ENABLE_DEBUG_LEDS

enum indicators {
    NUM_LOCK,
    CAPS_LOCK,
    SCROLL_LOCK,
    COMPOSE,
    KANA,
};

static bool raw_led_state[] = { [NUM_LOCK ... KANA] = false };

static inline void load_raw_led_state(void) {
    led_t host_led_state        = host_keyboard_led_state();
    raw_led_state[NUM_LOCK]     = host_led_state.num_lock;
    raw_led_state[CAPS_LOCK]    = host_led_state.caps_lock;
    raw_led_state[SCROLL_LOCK]  = host_led_state.scroll_lock;
    raw_led_state[COMPOSE]      = host_led_state.compose;
    raw_led_state[KANA]         = host_led_state.kana;
}

#ifdef LED_MATRIX_ENABLE
    static __attribute__((unused)) void set_led_off(uint16_t led_index) {
        led_matrix_set_value(led_index, 0);
    }

    static __attribute__((unused)) void set_led_on(uint16_t led_index) {
        led_matrix_set_value(led_index, 255);
    }
#else // RGB_MATRIX_ENABLE
    #include "utils/rgb.h"

    #ifdef LED_STATE_COLOR_FROM_BATTERY
        #include "utils/get_rgb_for_battery.h"

        // Cache LED RGB color calculation
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

#ifdef LED_STATE_RESIST_UNEXPECTED_CHANGES
    #if !defined(NUM_LOCK_INDEX) && !defined(CAPS_LOCK_INDEX) && !defined(SCROLL_LOCK_INDEX)
        #error "The LED state resist unexpected changes feature requires a num lock, caps lock, or scroll lock indicator to be enabled"
    #endif

    /*
     * Amount of time after a relevant key event for which we immediately accept LED state changes.
     * For example, if the caps lock key is pressed or released, we immediately accept any changes
     * to the caps lock LED state that occur during the next this many milliseconds.
     */
    #ifndef LED_STATE_KEY_EVENT_CHANGE_WINDOW_MS
        #define LED_STATE_KEY_EVENT_CHANGE_WINDOW_MS 20
    #endif

    /*
     * Amount of time that an unexpected LED state change (i.e. not apparently related to any key
     * event) must persist before we accept it. For example, if the operating system suddenly starts
     * reporting that the caps lock indicator changed state, the operating system must consistently
     * report that same value for the caps lock indicator for this many milliseconds before we
     * accept that change.
     */
    #ifndef LED_STATE_UNEXPECTED_CHANGE_THRESHOLD_MS
        #define LED_STATE_UNEXPECTED_CHANGE_THRESHOLD_MS 1000
    #endif

    // This should match the declaration of raw_led_state
    static bool cooked_led_state[] = { [NUM_LOCK ... KANA] = false };

    /*
     * To consider: Should we use the cooked LED state in LED_DRIVER_ALLOW_SHUTDOWN() in
     * keyboards/keychron/common/wireless/indicator.c? The problem is that I don't think we can
     * override it in user code--I think we would need to update the keyboard code. But I think that
     * function is only checked if the current RGB matrix effect is RGB_MATRIX_NONE (see
     * rgb_task_flush() in quantum/rgb_matrix/rgb_matrix.c), so it might not have any noticeable
     * impact.
     */

    #define TIMER_STOPPED 0
    /*
     * Guard against the rare case that we wrap around to exactly 0, because we use 0 to indicate
     * that the timer isn't running.
     */
    #define START_FAST_TIMER_FROM(now, expires_in) MAX(1, now + expires_in)
    #define START_FAST_TIMER(expires_in) START_FAST_TIMER_FROM(timer_read_fast(), expires_in)

    static fast_timer_t key_event_timers[]         = { [NUM_LOCK ... SCROLL_LOCK] = TIMER_STOPPED };
    static fast_timer_t unexpected_change_timers[] = { [NUM_LOCK ... SCROLL_LOCK] = TIMER_STOPPED };

    // timer_read_fast() seems to require a lock on some architectures, so cache it
    static fast_timer_t now;
    bool calculated_now = false;
    #define GET_NOW() (calculated_now ? now : (calculated_now = true, now = timer_read_fast()))
    #define RESET_NOW() (calculated_now = false)

    /*
     * This method assumes that load_raw_led_state() is called sometime before, because it uses
     * raw_led_state.
     * 
     * It also assumes that RESET_NOW() is called sometime beforehand, because it uses GET_NOW().
     */
    static void inline update_cooked_led_state_for_indicator(size_t indicator_index) {
        // A key event occurred recently, so accept any state changes immediately
        if (key_event_timers[indicator_index] != TIMER_STOPPED) {
            if (!timer_expired_fast(GET_NOW(), key_event_timers[indicator_index])) {
                cooked_led_state[indicator_index] = raw_led_state[indicator_index];
            } else {
                key_event_timers[indicator_index] = TIMER_STOPPED;
            }
        }

        // No key events occurred recently, so check for unexpected changes
        if (key_event_timers[indicator_index] == TIMER_STOPPED) {
            if (raw_led_state[indicator_index] != cooked_led_state[indicator_index]) {
                // An unexpected change is present, so start a timer, or accept the state change if
                // it's still present at the timer's expiration
                if (unexpected_change_timers[indicator_index] == TIMER_STOPPED) {
                    unexpected_change_timers[indicator_index] = START_FAST_TIMER_FROM(GET_NOW(),
                            LED_STATE_UNEXPECTED_CHANGE_THRESHOLD_MS);
                } else if (timer_expired_fast(GET_NOW(), unexpected_change_timers[indicator_index])) {
                    cooked_led_state[indicator_index] = raw_led_state[indicator_index];
                    unexpected_change_timers[indicator_index] = TIMER_STOPPED;
                }
            } else {
                // If timer was running, stop it if raw state reverts back to cooked state
                unexpected_change_timers[indicator_index] = TIMER_STOPPED;
            }
        }
    }

    static void update_cooked_led_state(void) {
        load_raw_led_state();

        // Don't resist unexpected changes for the composed and kana state because I don't use that
        // functionality, so I don't fully understand it nor do I have a good way to test it.
        #ifdef COMPOSE_LOCK_INDEX
            cooked_led_state[COMPOSE] = raw_led_state[COMPOSE];
        #endif
        #ifdef KANA_LOCK_INDEX
            cooked_led_state[KANA] = raw_led_state[KANA];
        #endif

        // Recalculate "now" the first time we need it during this invocation
        RESET_NOW();

        #ifdef NUM_LOCK_INDEX
            update_cooked_led_state_for_indicator(NUM_LOCK);
        #endif
        #ifdef CAPS_LOCK_INDEX
            update_cooked_led_state_for_indicator(CAPS_LOCK);
        #endif
        #ifdef SCROLL_LOCK_INDEX
            update_cooked_led_state_for_indicator(SCROLL_LOCK);
        #endif
    }

    static inline void process_key_event_for_indicator(size_t indicator_index) {
        key_event_timers[indicator_index] = START_FAST_TIMER(LED_STATE_KEY_EVENT_CHANGE_WINDOW_MS);
        unexpected_change_timers[indicator_index] = TIMER_STOPPED;
    }
    
    bool process_record_led_state_indicators(uint16_t keycode, keyrecord_t *record) {
        /*
         * We start the timer on both key press and key release events because _either_ could
         * trigger a state change. For example, in USB mode the state seems to change on key press;
         * whereas in 2.4 GHz mode the state seems to change on key release (or after the key has
         * been held down for some amount of time).
         */
        switch (keycode) {
            #ifdef NUM_LOCK_INDEX
                case KC_NUM_LOCK:
                    process_key_event_for_indicator(NUM_LOCK);
                    break;
            #endif
            #ifdef CAPS_LOCK_INDEX
                case KC_CAPS_LOCK:
                    process_key_event_for_indicator(CAPS_LOCK);
                    break;
            #endif
            #ifdef SCROLL_LOCK_INDEX
                case KC_SCROLL_LOCK:
                    process_key_event_for_indicator(SCROLL_LOCK);
                    break;
            #endif
        }
        return true; // Continue processing
    }
#endif // LED_STATE_RESIST_UNEXPECTED_CHANGES

// Set LEDs based on the given state
static void set_leds_from_state(bool led_state[5]) {
    #if defined(LED_STATE_COLOR_FROM_BATTERY) && defined(RGB_MATRIX_ENABLE)
        // Recalculate led_rgb_color the first time we need it during this invocation
        calculated_led_rgb_color = false;
    #endif

    #if defined(NUM_LOCK_INDEX)
        if (led_state[NUM_LOCK]) {
            set_led_on(NUM_LOCK_INDEX);
        }
    #endif

    #if defined(CAPS_LOCK_INDEX)
        if (led_state[CAPS_LOCK]) {
            #if defined(DIM_CAPS_LOCK)
                set_led_off(CAPS_LOCK_INDEX);
            #else
                set_led_on(CAPS_LOCK_INDEX);
            #endif
        }
    #endif

    #if defined(SCROLL_LOCK_INDEX)
        if (led_state[SCROLL_LOCK]) {
            set_led_on(SCROLL_LOCK_INDEX);
        }
    #endif

    #if defined(COMPOSE_LOCK_INDEX)
        if (led_state[COMPOSE]) {
            set_led_on(COMPOSE_LOCK_INDEX);
        }
    #endif

    #if defined(KANA_LOCK_INDEX)
        if (led_state[KANA]) {
            set_led_on(KANA_LOCK_INDEX);
        }
    #endif
}

#ifdef ENABLE_DEBUG_LEDS
    #ifdef LED_MATRIX_ENABLE
        #define SET_DEBUG_LED_ON(led_index) led_matrix_set_value(led_index, 255)
    #else // RGB_MATRIX_ENABLE
        #define SET_DEBUG_LED_ON(led_index) rgb_matrix_set_color(led_index, 255, 255, 255)
    #endif

    // Use LEDs of number keys 1-8 to display the bits of the raw LED state
    static void set_debug_leds(void) {
        uint8_t raw_bits = host_keyboard_led_state().raw;
        if (raw_bits & (1<<7)) { // reserved bit
            SET_DEBUG_LED_ON(17); // 1 key
        }
        if (raw_bits & (1<<6)) { // reserved bit
            SET_DEBUG_LED_ON(18); // 2 key
        }
        if (raw_bits & (1<<5)) { // reserved bit
            SET_DEBUG_LED_ON(19); // 3 key
        }
        if (raw_bits & (1<<4)) { // kana bit
            SET_DEBUG_LED_ON(20); // 4 key
        }
        if (raw_bits & (1<<3)) { // compose bit
            SET_DEBUG_LED_ON(21); // 5 key
        }
        if (raw_bits & (1<<2)) { // scroll_lock bit
            SET_DEBUG_LED_ON(22); // 6 key
        }
        if (raw_bits & (1<<1)) { // caps_lock bit
            SET_DEBUG_LED_ON(23); // 7 key
        }
        if (raw_bits & (1<<0)) { // num_lock bit
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

    #ifdef LED_STATE_RESIST_UNEXPECTED_CHANGES
        update_cooked_led_state();
        set_leds_from_state(cooked_led_state);
    #else
        load_raw_led_state();
        set_leds_from_state(raw_led_state);
    #endif

    #ifdef ENABLE_DEBUG_LEDS
        set_debug_leds();
    #endif
}

#endif // (LED_STATE_COLOR_FROM_BATTERY || LED_STATE_RESIST_UNEXPECTED_CHANGES) && (LED_MATRIX_ENABLE || RGB_MATRIX_ENABLE)
