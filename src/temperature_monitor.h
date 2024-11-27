#ifndef TEMPERATURE_MONITOR_H
#define TEMPERATURE_MONITOR_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 10U
#define SENSOR_READ_INTERVAL_SEC 1U
#define DISPLAY_INTERVAL_SEC 5U

typedef struct {
    float temperatures[BUFFER_SIZE];
    uint32_t write_index;
    uint32_t read_index;
    uint32_t count;
    pthread_mutex_t mutex;
} CircularBuffer_t;

typedef struct {
    CircularBuffer_t* buffer;
    volatile bool* running;
} ThreadArgs_t;

void circular_buffer_init(CircularBuffer_t* const buffer);
bool circular_buffer_write(CircularBuffer_t* const buffer, const float value);
bool circular_buffer_read(CircularBuffer_t* const buffer, float* const value);
float simulate_temperature_reading(void);
void display_buffer_contents(CircularBuffer_t* const buffer);
void* sensor_thread(void* arg);
void* display_thread(void* arg);
void sleep_until_next_period(struct timespec* next_period, time_t interval_sec);

#endif // TEMPERATURE_MONITOR_H