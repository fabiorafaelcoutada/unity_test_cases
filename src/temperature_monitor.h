#ifndef TEMPERATURE_MONITOR_H
#define MUCH_H

#include <stdint.h>
#include <stdbool.h>
#include <threads.h>
#include <time.h>
#include <stdatomic.h>

#define BUFFER_SIZE 10U
#define SENSOR_READ_INTERVAL_SEC 1U
#define DISPLAY_INTERVAL_SEC 5U

// Enumeration for different error types
typedef enum {
    ERROR_NONE = 0,
    ERROR_BUFFER_OVERFLOW,
    ERROR_BUFFER_UNDERFLOW
} BufferError_t;

typedef struct {
    float temperatures[BUFFER_SIZE];
    uint32_t write_index;
    uint32_t read_index;
    uint32_t count;
    mtx_t mutex;
    BufferError_t last_error;
} CircularBuffer_t;

typedef struct {
    CircularBuffer_t* buffer;
    atomic_bool* running;
} ThreadArgs_t;

void circular_buffer_init(CircularBuffer_t* const buffer);
bool circular_buffer_write(CircularBuffer_t* const buffer, const float value);
bool circular_buffer_read(CircularBuffer_t* const buffer, float* const value);
void display_buffer_error(CircularBuffer_t* const buffer);
float simulate_temperature_reading(void);
void display_buffer_contents(CircularBuffer_t* const buffer);
int sensor_thread(void* arg);
int display_thread(void* arg);
void sleep_until_next_period(struct timespec* next_period, time_t interval_sec);

#endif // TEMPERATURE_MONITOR_H