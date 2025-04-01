/**
* timing.h
* Provides functions for simulating CPU activity with microsecond precision
*/

#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

/**
* Different work timing modes
*/
typedef enum {
    BUSY_WAIT_ACCURATE,   // Most accurate, highest CPU usage
    BUSY_WAIT_EFFICIENT,  // Less accurate, still high CPU usage
    YIELD_WAIT,           // Yields to other threads, medium accuracy
    HYBRID_WAIT           // Combination of busy wait and yield
} timing_mode_t;

/**
* Configuration for the timing
*/
typedef struct {
    timing_mode_t mode;    // timing mode
    uint32_t check_frequency;  // How often to check time (iterations)
    bool verbose;              // Print diagnostic information
} timing_config_t;

/**
* Statistics from a timing run
*/
typedef struct {
    uint64_t iterations;       // Number of iterations performed
    uint64_t time_checks;      // Number of time checks performed
    uint64_t requested_us;     // Requested duration in microseconds
    uint64_t actual_us;        // Actual duration in microseconds
    double accuracy_percent;   // Accuracy as a percentage
} timing_stats_t;

/**
* Function to get time in nanoseconds
* @return current time in nanoseconds
*/
uint64_t get_time_ns(void);

/**
* Initialize the timing library
* @return true if initialization succeeded, false otherwise
*/
bool timing_init(void);

/**
* Clean up the timing library
*/
void timing_cleanup(void);

/**
* Get the default timing configuration
* @return Default configuration
*/
timing_config_t timing_get_default_config(void);

/**
* Set the global timing configuration
* @param config The configuration to set
*/
void timing_set_config(timing_config_t config);

/**
* Get the current timing configuration
* @return Current configuration
*/
timing_config_t timing_get_config(void);

/**
* Busy wait for the specified number of microseconds
* 
* @param microseconds Duration to wait in microseconds
* @return Statistics about the timing run
*/
timing_stats_t timing_busy_wait_us(uint64_t microseconds);

/**
* Busy wait for the specified number of milliseconds
* 
* @param milliseconds Duration to wait in milliseconds
* @return Statistics about the timing run
*/
timing_stats_t timing_busy_wait_ms(uint64_t milliseconds);

/**
* Busy wait for the specified number of seconds
* 
* @param seconds Duration to wait in seconds
* @return Statistics about the timing run
*/
timing_stats_t timing_busy_wait_s(double seconds);


/**
* Print statistics from a timing run
* 
* @param stats Statistics to print
*/
void timing_print_stats(timing_stats_t stats);

#endif /* TIMING_H */