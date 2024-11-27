#include "temperature_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void circular_buffer_init(CircularBuffer_t* const buffer) {
    if (NULL == buffer) {
        return;
    }

    buffer->write_index = 0U;
    buffer->read_index = 0U;
    buffer->count = 0U;
    pthread_mutex_init(&buffer->mutex, NULL);
}

bool circular_buffer_write(CircularBuffer_t* const buffer, const float value) {
    bool result = false;

    if (NULL == buffer) {
        return false;
    }

    pthread_mutex_lock(&buffer->mutex);

    if (buffer->count < BUFFER_SIZE) {
        buffer->temperatures[buffer->write_index] = value;
        buffer->write_index = (buffer->write_index + 1U) % BUFFER_SIZE;
        buffer->count++;
        result = true;
    }

    pthread_mutex_unlock(&buffer->mutex);
    return result;
}

bool circular_buffer_read(CircularBuffer_t* const buffer, float* const value) {
    bool result = false;

    if ((NULL == buffer) || (NULL == value)) {
        return false;
    }

    pthread_mutex_lock(&buffer->mutex);

    if (buffer->count > 0U) {
        *value = buffer->temperatures[buffer->read_index];
        buffer->read_index = (buffer->read_index + 1U) % BUFFER_SIZE;
        buffer->count--;
        result = true;
    }

    pthread_mutex_unlock(&buffer->mutex);
    return result;
}

void display_buffer_contents(CircularBuffer_t* const buffer) {
    float temperature;
    uint32_t reading_count = 0U;

    if (NULL == buffer) {
        return;
    }

    printf("\nCurrent Buffer Contents:\n");

    while (circular_buffer_read(buffer, &temperature)) {
        printf("Reading %u: %.2f°C\n", ++reading_count, temperature);
    }

    if (0U == reading_count) {
        printf("Buffer is empty\n");
    }
}

float simulate_temperature_reading(void) {
    return 20.0F + ((float)rand() / (float)RAND_MAX) * 10.0F;
}

void sleep_until_next_period(struct timespec* next_period, time_t interval_sec) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, next_period, NULL);
    next_period->tv_sec += interval_sec;
}

void* sensor_thread(void* arg) {
    ThreadArgs_t* thread_args = (ThreadArgs_t*)arg;
    struct timespec next_period;
    time_t current_time;
    struct tm* time_info;

    clock_gettime(CLOCK_MONOTONIC, &next_period);

    while (*(thread_args->running)) {
        float temperature = simulate_temperature_reading();

        if (circular_buffer_write(thread_args->buffer, temperature)) {
            current_time = time(NULL);
            time_info = localtime(&current_time);
            printf("[%02d:%02d:%02d] Temperature logged: %.2f°C\n",
                   time_info->tm_hour,
                   time_info->tm_min,
                   time_info->tm_sec,
                   temperature);
        }

        sleep_until_next_period(&next_period, SENSOR_READ_INTERVAL_SEC);
    }

    return NULL;
}

void* display_thread(void* arg) {
    ThreadArgs_t* thread_args = (ThreadArgs_t*)arg;
    struct timespec next_period;

    clock_gettime(CLOCK_MONOTONIC, &next_period);

    while (*(thread_args->running)) {
        display_buffer_contents(thread_args->buffer);

        sleep_until_next_period(&next_period, DISPLAY_INTERVAL_SEC);
    }

    return NULL;
}