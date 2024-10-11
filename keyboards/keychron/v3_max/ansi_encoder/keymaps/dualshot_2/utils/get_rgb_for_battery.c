#include "battery.h"

#include "utils/get_rgb_for_battery.h"

/*
 * To consider: We could cache the value for one or several minutes if we think the battery
 *   percentage calculation is expensive (relative to a timer check).
 */
struct rgb get_rgb_for_battery(void) {
    uint8_t battery_percentage = battery_get_percentage(); // Range: [0, 100]
    if (battery_percentage > 75) {             // (75, 100]
        return (struct rgb) {  96, 255,   0 }; //  Green
    } else if (battery_percentage > 50) {      // (50, 75]
        return (struct rgb) { 255, 160,   0 }; //  Yellow
    } else if (battery_percentage > 25) {      // (25, 50]
        return (struct rgb) { 255,  36,   0 }; //  Orange
    } else {                                   // [0, 25]
        return (struct rgb) { 255,   4,   4 }; //  Red
    }
}
