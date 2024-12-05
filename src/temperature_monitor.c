#include "temperature_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <threads.h>

void circular_buffer_init(CircularBuffer_t* const buffer) {
    if (buffer == NULL) {
        return;
    }

    buffer->write_index = 0U;
    buffer->read_index = 0U;
    buffer->count = 0U;
    mtx_init(&buffer->mutex, mtx_plain);
}

bool circular_buffer_write(CircularBuffer_t* const buffer, const float value) {
    bool result = false;

    if (buffer == NULL) {
        return false;
    }

    mtx_lock(&buffer->mutex);

    if (buffer->count < BUFFER_SIZE) {
        // Normal write when buffer is not full
        buffer->temperatures[buffer->write_index] = value;
        buffer->write_index = (buffer->write_index + 1U) % BUFFER_SIZE;
        buffer->count++;
        result = true;
    } else {
        // When buffer is full, overwrite the oldest data
        buffer->temperatures[buffer->write_index] = value;
        buffer->write_index = (buffer->write_index + 1U) % BUFFER_SIZE;
        buffer->read_index = (buffer->read_index + 1U) % BUFFER_SIZE;

        // Error tracking is optional, uncomment if needed
        // buffer->last_error = ERROR_BUFFER_OVERFLOW;
        // fprintf(stderr, "Buffer full: Overwriting oldest data\n");

        result = true;
    }

    mtx_unlock(&buffer->mutex);
    return result;
}
bool circular_buffer_read(CircularBuffer_t* const buffer, float* const value) {
    bool result = false;

    if ((buffer == NULL) || (value == NULL)) {
        return false;
    }

    mtx_lock(&buffer->mutex);

    if (buffer->count > 0U) {
        *value = buffer->temperatures[buffer->read_index];
        buffer->read_index = (buffer->read_index + 1U) % BUFFER_SIZE;
        buffer->count--;
        result = true;
    } else {
        buffer->last_error = ERROR_BUFFER_UNDERFLOW;
        fprintf(stderr, "Error: Buffer underflow - No temperatures to read\n");
    }

    mtx_unlock(&buffer->mutex);
    return result;
}

void display_buffer_contents(CircularBuffer_t* const buffer) {
    if (buffer == NULL) {
        return;
    }

    printf("\nCurrent Buffer Contents:\n");

    mtx_lock(&buffer->mutex);

    if (buffer->count == 0) {
        printf("Buffer is empty\n");
    } else {
        uint32_t current_index = buffer->read_index;
        for (uint32_t i = 0; i < buffer->count; i++) {
            printf("Reading %u: %.2f°C\n", i + 1, buffer->temperatures[current_index]);
            current_index = (current_index + 1U) % BUFFER_SIZE;
        }
    }

    mtx_unlock(&buffer->mutex);
}

void display_buffer_error(CircularBuffer_t* const buffer) {
    if (buffer == NULL) {
        return;
    }

    switch (buffer->last_error) {
        case ERROR_BUFFER_OVERFLOW:
            fprintf(stderr, "Last error: Buffer Overflow\n");
        break;
        case ERROR_BUFFER_UNDERFLOW:
            fprintf(stderr, "Last error: Buffer Underflow\n");
        break;
        case ERROR_NONE:
        default:
            printf("No errors detected\n");
        break;
    }
}

float simulate_temperature_reading(void) {
    return 20.0F + ((float)rand() / (float)RAND_MAX) * 10.0F;
}

int sensor_thread(void* arg) {
    ThreadArgs_t* thread_args = (ThreadArgs_t*)arg;
    time_t current_time;
    struct tm* time_info;

    while (atomic_load(thread_args->running)) {
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

        thrd_sleep(&(struct timespec){.tv_sec = SENSOR_READ_INTERVAL_SEC}, NULL);
    }

    return 0;
}

int display_thread(void* arg) {
    ThreadArgs_t* thread_args = (ThreadArgs_t*)arg;

    while (atomic_load(thread_args->running)) {
        display_buffer_contents(thread_args->buffer);
        display_buffer_error(thread_args->buffer);

        thrd_sleep(&(struct timespec){.tv_sec = DISPLAY_INTERVAL_SEC}, NULL);
    }

    return 0;
}