#include "unity.h"
#include "temperature_monitor.h"
#include <string.h>

CircularBuffer_t test_buffer;
float test_readings[15] = {
    21.5F, 22.5F, 23.5F, 24.5F, 25.5F,
    26.5F, 27.5F, 28.5F, 29.5F, 30.5F,
    31.5F, 32.5F, 33.5F, 34.5F, 35.5F
};

void setUp(void) {
    circular_buffer_init(&test_buffer);
}

void tearDown(void) {
    pthread_mutex_destroy(&test_buffer.mutex);
}

void test_buffer_initialization(void) {
    CircularBuffer_t buffer;
    circular_buffer_init(&buffer);

    TEST_ASSERT_EQUAL_UINT32(0, buffer.count);
    TEST_ASSERT_EQUAL_UINT32(0, buffer.read_index);
    TEST_ASSERT_EQUAL_UINT32(0, buffer.write_index);

    pthread_mutex_destroy(&buffer.mutex);
}

void test_single_write_read(void) {
    float read_value;

    TEST_ASSERT_TRUE(circular_buffer_write(&test_buffer, test_readings[0]));
    TEST_ASSERT_EQUAL_UINT32(1, test_buffer.count);

    TEST_ASSERT_TRUE(circular_buffer_read(&test_buffer, &read_value));
    TEST_ASSERT_EQUAL_FLOAT(test_readings[0], read_value);
    TEST_ASSERT_EQUAL_UINT32(0, test_buffer.count);
}

void test_multiple_writes_until_full(void) {
    uint32_t i;
    float read_value;

    // Fill buffer
    for (i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_TRUE(circular_buffer_write(&test_buffer, test_readings[i]));
        TEST_ASSERT_EQUAL_UINT32(i + 1, test_buffer.count);
    }

    // Verify all values
    for (i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_TRUE(circular_buffer_read(&test_buffer, &read_value));
        TEST_ASSERT_EQUAL_FLOAT(test_readings[i], read_value);
    }
}

void test_circular_overwrite(void) {
    uint32_t i;
    float read_value;

    // Fill buffer and add 5 more readings (overwriting first 5)
    for (i = 0; i < BUFFER_SIZE + 5; i++) {
        TEST_ASSERT_TRUE(circular_buffer_write(&test_buffer, test_readings[i]));
    }

    TEST_ASSERT_EQUAL_UINT32(BUFFER_SIZE, test_buffer.count);

    // Read all values and verify we get the last 10 readings
    for (i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_TRUE(circular_buffer_read(&test_buffer, &read_value));
        TEST_ASSERT_EQUAL_FLOAT(test_readings[i + 5], read_value);
    }

    TEST_ASSERT_EQUAL_UINT32(0, test_buffer.count);
}

void test_read_empty_buffer(void) {
    float read_value;

    TEST_ASSERT_FALSE(circular_buffer_read(&test_buffer, &read_value));
    TEST_ASSERT_EQUAL_UINT32(0, test_buffer.count);
}

void test_write_null_buffer(void) {
    TEST_ASSERT_FALSE(circular_buffer_write(NULL, 25.0F));
}

void test_read_null_buffer(void) {
    float read_value;
    TEST_ASSERT_FALSE(circular_buffer_read(NULL, &read_value));
}

void test_read_null_value_pointer(void) {
    circular_buffer_write(&test_buffer, 25.0F);
    TEST_ASSERT_FALSE(circular_buffer_read(&test_buffer, NULL));
}

void test_alternating_read_write(void) {
    float read_value;
    uint32_t i;

    for (i = 0; i < 5; i++) {
        TEST_ASSERT_TRUE(circular_buffer_write(&test_buffer, test_readings[i]));
        TEST_ASSERT_TRUE(circular_buffer_read(&test_buffer, &read_value));
        TEST_ASSERT_EQUAL_FLOAT(test_readings[i], read_value);
        TEST_ASSERT_EQUAL_UINT32(0, test_buffer.count);
    }
}

void test_write_full_cycle(void) {
    uint32_t i;
    float read_value;

    // Write enough values to wrap around the buffer multiple times
    for (i = 0; i < BUFFER_SIZE * 3; i++) {
        TEST_ASSERT_TRUE(circular_buffer_write(&test_buffer, test_readings[i % 15]));
    }

    TEST_ASSERT_EQUAL_UINT32(BUFFER_SIZE, test_buffer.count);

    // Verify we can read the last BUFFER_SIZE values
    for (i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_TRUE(circular_buffer_read(&test_buffer, &read_value));
        TEST_ASSERT_EQUAL_FLOAT(test_readings[(BUFFER_SIZE * 3 - BUFFER_SIZE + i) % 15], read_value);
    }
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_buffer_initialization);
    RUN_TEST(test_single_write_read);
    RUN_TEST(test_multiple_writes_until_full);
    RUN_TEST(test_circular_overwrite);
    RUN_TEST(test_read_empty_buffer);
    RUN_TEST(test_write_null_buffer);
    RUN_TEST(test_read_null_buffer);
    RUN_TEST(test_read_null_value_pointer);
    RUN_TEST(test_alternating_read_write);
    RUN_TEST(test_write_full_cycle);

    return UNITY_END();
}