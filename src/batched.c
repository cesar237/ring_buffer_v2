/**
* Example usage of the MPMC ring buffer with command line arguments
*/

#define _GNU_SOURCE

#include "ring_buffer.h"
#include "timing.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Default values
#define DEFAULT_BUFFER_SIZE 1024
#define DEFAULT_NUM_PRODUCERS 1
#define DEFAULT_NUM_CONSUMERS 4
#define DEFAULT_SERVICE_TIME 10
#define DEFAULT_DURATION 10
#define BURST 1
#define PIN_THREADS true
#define DEFAULT_BATCH_SIZE 1


typedef struct {
    uint64_t produce_time;
    uint64_t consume_time;
    int producer_id;
    int consumer_id;
    uint64_t id;
} test_item_t;

typedef struct {
    int id;
    bool pin_thread;
    int core;
    int total_produced;
    test_item_t *items;
    ring_buffer_t *buffer;
    uint64_t total_spin_time;
    uint64_t total_service_time;
    uint64_t total_running_time;
    int num_producers;
    uint64_t duration;
    int burst;
} producer_args_t;

typedef struct {
    int id;
    bool pin_thread;
    int core;
    int total_consumed;
    test_item_t *items;
    ring_buffer_t *buffer;
    uint64_t duration;
    uint64_t total_spin_time;
    uint64_t total_service_time;
    uint64_t total_running_time;
    int service_time;
    int num_consumers;
    int batch_size;
    uint64_t latencies[100000000];
} consumer_args_t;

/**
* Pin the current thread to a specific CPU core.
* 
* @param core_id The core ID to pin the thread to (starting from 0)
* @return 0 on success, -1 on failure
*/
bool pin_thread_to_core(int core_id) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    
    if (core_id < 0 || core_id >= num_cores) {
        fprintf(stderr, "Error: Core ID %d is out of range (0-%d)\n", 
                core_id, num_cores - 1);
        return false;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_t current_thread = pthread_self();
    int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to set thread affinity\n");
        return false;
    }
    
    return true;
}

void* producer_thread(void* arg) {
    producer_args_t* producer_arg = (producer_args_t*)arg;

    if (producer_arg->pin_thread && !pin_thread_to_core(producer_arg->core))
        return NULL;

    // For the duration of the simulation, produce items
    uint64_t start = get_time_ns();

    while ((get_time_ns() - start) / 1000000000 < producer_arg->duration) {
        // usleep(1); // Simulate some work before producing the item
        uint64_t items[128];
        uint64_t timestamp = get_time_ns();
        for (int i = 0; i < producer_arg->burst; i++) 
        items[i] = timestamp + i + 1;

        uint64_t start_spin = get_time_ns();
        int got = ring_buffer_produce_batch(producer_arg->buffer, (void **)items, producer_arg->burst);
        uint64_t end_spin = get_time_ns();
        producer_arg->total_spin_time += end_spin - start_spin;
        producer_arg->total_produced += got;
        if (got == 0)
            continue;
    }

    uint64_t end = get_time_ns();
    producer_arg->total_running_time = end - start;
    return NULL;
}

/**
* Consumer thread function
*/
void* consumer_thread(void* arg) {
    consumer_args_t* consumer_arg = (consumer_args_t*)arg;

    if (consumer_arg->pin_thread && !pin_thread_to_core(consumer_arg->core))
        return NULL;

    // For the duration of the simulation, consume items
    uint64_t start = get_time_ns();
    // uint64_t end = start + consumer_arg->duration;

    // pring duration time
    printf("Consumer %d duration time: %lu\n", consumer_arg->id, consumer_arg->duration);

    while ((get_time_ns() - start) / 1000000000 < consumer_arg->duration) {
        uint64_t items[64];

        uint64_t start_spin = get_time_ns();
        int got = ring_buffer_consume_batch(consumer_arg->buffer, (void **)items, consumer_arg->batch_size);
        uint64_t end_spin = get_time_ns();

        if (got == 0) {
            // No item produced yet
            continue;
        }
        for (int j = 0; j < got; j++) {
            // simulate service time
            timing_busy_wait_us(consumer_arg->service_time);

            consumer_arg->latencies[consumer_arg->total_consumed] = get_time_ns() - items[j];
            consumer_arg->total_consumed += 1;
        }
        consumer_arg->total_spin_time += end_spin - start_spin;
        consumer_arg->total_service_time += consumer_arg->service_time * got;
    }
    consumer_arg->total_running_time = (get_time_ns() - start);
    return NULL;
}

int main(int argc, char *argv[]) {
    int num_producers = DEFAULT_NUM_PRODUCERS;
    int num_consumers = DEFAULT_NUM_CONSUMERS;
    int service_time = DEFAULT_SERVICE_TIME;
    int duration = DEFAULT_DURATION;
    int burst = BURST;
    int batch_size = DEFAULT_BATCH_SIZE;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc) { // Make sure we have a value after the parameter
            if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--producers") == 0) {
                num_producers = atoi(argv[i + 1]);
                i++; // Skip the value in the next iteration
            } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--consumers") == 0) {
                num_consumers = atoi(argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--service-time") == 0) {
                service_time = atoi(argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--duration") == 0) {
                duration = atoi(argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--burst") == 0) {
                burst = atoi(argv[i + 1]);
                i++;
            }
            else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--batch") == 0) {
                batch_size = atoi(argv[i + 1]);
                i++;
            }
        }
    }
    
    // Just printing to verify the parsed values
    printf("Configuration:\n");
    printf("  Number of producers: %d\n", num_producers);
    printf("  Number of consumers: %d\n", num_consumers);
    printf("  Service time: %d\n", service_time);
    printf("  Duration: %d\n", duration);
    printf("  Burst: %d\n", burst);
    printf("  Batch size: %d\n", batch_size);

    ring_buffer_t buffer;
    if (!ring_buffer_init_batch(&buffer, DEFAULT_BUFFER_SIZE, sizeof(test_item_t), burst)) {
        fprintf(stderr, "Failed to initialize ring buffer\n");
        return 1;
    }
    // Allocate memory for producer and consumer threads    
    pthread_t* producers = (pthread_t*)malloc(num_producers * sizeof(pthread_t));
    pthread_t* consumers = (pthread_t*)malloc(num_consumers * sizeof(pthread_t));
    producer_args_t* producer_args = (producer_args_t*)malloc(num_producers * sizeof(producer_args_t));
    consumer_args_t* consumer_args = (consumer_args_t*)malloc(num_consumers * sizeof(consumer_args_t));
    
    if (!producers || !consumers || !producer_args || !consumer_args) {
        fprintf(stderr, "Failed to allocate memory for threads\n");
        ring_buffer_destroy(&buffer);
        free(producers);
        free(consumers);
        free(producer_args);
        free(consumer_args);
        return 1;
    }

    // Create producer threads
    for (int i = 0; i < num_producers; i++) {
        producer_args[i].id = i + 1;
        producer_args[i].core = i % sysconf(_SC_NPROCESSORS_ONLN); // Distribute across available cores
        producer_args[i].total_produced = 0;
        
        producer_args[i].buffer = &buffer;
        producer_args[i].burst = burst;
        producer_args[i].total_spin_time = 0;
        producer_args[i].total_service_time = 0;
        producer_args[i].total_running_time = 0;
        producer_args[i].num_producers = num_producers;
        producer_args[i].duration = duration;
        producer_args[i].pin_thread = PIN_THREADS; // Pin thread to core

        if (pthread_create(&producers[i], NULL, producer_thread, &producer_args[i]) != 0) {
            fprintf(stderr, "Failed to create producer thread %d\n", i + 1);
            ring_buffer_destroy(&buffer);
            free(producers);
            free(consumers);
            free(producer_args);
            free(consumer_args);
            return 1;
        }
    }
    
    // Create consumer threads
    for (int i = 0; i < num_consumers; i++) {
        consumer_args[i].id = i + 1;
        consumer_args[i].core = (i + num_producers) % sysconf(_SC_NPROCESSORS_ONLN); // Distribute across available cores
        consumer_args[i].total_consumed = 0;
        consumer_args[i].buffer = &buffer;
        consumer_args[i].total_spin_time = 0;
        consumer_args[i].total_service_time = 0;
        consumer_args[i].total_running_time = 0;
        consumer_args[i].service_time = service_time;
        consumer_args[i].num_consumers = num_consumers;
        consumer_args[i].duration = duration;
        consumer_args[i].pin_thread = PIN_THREADS; // Pin thread to core
        consumer_args[i].batch_size = batch_size;

        if (pthread_create(&consumers[i], NULL, consumer_thread, &consumer_args[i]) != 0) {
            fprintf(stderr, "Failed to create consumer thread %d\n", i + 1);
            ring_buffer_destroy(&buffer);
            free(producers);
            free(consumers);
            free(producer_args);
            free(consumer_args);
            return 1;
        }
    }
    
    // Wait for producer threads to finish
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for consumer threads to finish
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Print statistics
    printf("Producer Statistics:\n");
    uint64_t total_produced = 0;
    for (int i = 0; i < num_producers; i++) {
        total_produced += producer_args[i].total_produced;
        printf("  Producer %d:\n", producer_args[i].id);
        printf("    Total produced: %d\n", producer_args[i].total_produced);
        printf("    Total running time: %lu ns\n", producer_args[i].total_running_time);
        printf("    Total service time: %lu ns\n", producer_args[i].total_service_time);
        printf("    Total spin time: %lu ns\n", producer_args[i].total_spin_time);
    }

    printf("Consumer Statistics in second:\n");
    uint64_t total_consumed = 0;
    uint64_t total_service_time = 0;
    uint64_t total_spin_time = 0;
    double total_latency = 0;
    for (int i = 0; i < num_consumers; i++) {
        total_consumed += consumer_args[i].total_consumed;
        total_service_time += consumer_args[i].total_service_time;
        total_spin_time += consumer_args[i].total_spin_time;

        printf("  Consumer %d:\n", consumer_args[i].id);
        printf("    Total consumed: %d\n", consumer_args[i].total_consumed);
        printf("    Total running time: %.2f ms\n", consumer_args[i].total_running_time / 1000000.0);
        printf("    Total service time: %.2f ms\n", consumer_args[i].total_service_time / 1000.0);
        printf("    Total spin time: %.2f ms\n", consumer_args[i].total_spin_time / 1000000.0);

        double avg_latency = 0;
        for (int j = 0; j < consumer_args[i].total_consumed; j++) {
            avg_latency += consumer_args[i].latencies[j];
        }
        avg_latency /= consumer_args[i].total_consumed;
        avg_latency /= 1000.0;
        total_latency += avg_latency;
        printf("    Average latency: %.2f us\n", avg_latency);
    }

    printf("\nTotal produced: %lu\n", total_produced);
    printf("Total consumed: %lu\n", total_consumed);
    printf("Difference: %lu\n", total_produced - total_consumed);
    printf("Total service time: %.2f ms\n", total_service_time / 1000.0);
    printf("Total spin time: %.2f ms\n", total_spin_time / 1000000.0);
    printf("Average latency: %.2f us\n", total_latency / num_consumers);

    // clean up
    for (int i = 0; i < num_producers; i++) {
        free(producer_args[i].items);
    }
    for (int i = 0; i < num_consumers; i++) {
        free(consumer_args[i].items);
    }
    free(producers);
    free(consumers);
    free(producer_args);
    free(consumer_args);
    ring_buffer_destroy(&buffer);

    return 0;
}