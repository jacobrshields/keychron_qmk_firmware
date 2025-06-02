#pragma once

#define TIMER_STOPPED 0
/*
 * Guard against the rare case that we wrap around to exactly 0, because we use 0 to indicate
 * that the timer isn't running.
 */
#define START_FAST_TIMER_FROM(now, expires_in) MAX(1, now + expires_in)
#define START_FAST_TIMER(expires_in) START_FAST_TIMER_FROM(timer_read_fast(), expires_in)
