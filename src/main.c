#include "temperature_monitor.h"
#include <stdlib.h>
#include <threads.h>
#include <stdio.h>
int main(void) {
    CircularBuffer_t buffer;
    atomic_bool running = ATOMIC_VAR_INIT(true);
    
    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Initialize circular buffer
    circular_buffer_init(&buffer);

    // Thread arguments
    ThreadArgs_t sensor_args = { &buffer, &running };
    ThreadArgs_t display_args = { &buffer, &running };

    // Thread handles
    thrd_t sensor_thrd, display_thrd;

    // Create threads
    if (thrd_create(&sensor_thrd, sensor_thread, &sensor_args) != thrd_success) {
        fprintf(stderr, "Failed to create sensor thread\n");
        return EXIT_FAILURE;
    }

    if (thrd_create(&display_thrd, display_thread, &display_args) != thrd_success) {
        fprintf(stderr, "Failed to create display thread\n");
        atomic_store(&running, false);
        thrd_join(sensor_thrd, NULL);
        return EXIT_FAILURE;
    }

    // Simulate running for a while (e.g., 30 seconds)
    thrd_sleep(&(struct timespec){.tv_sec = 30}, NULL);

    // Stop threads
    atomic_store(&running, false);

    // Wait for threads to finish
    thrd_join(sensor_thrd, NULL);
    thrd_join(display_thrd, NULL);

    return EXIT_SUCCESS;
}