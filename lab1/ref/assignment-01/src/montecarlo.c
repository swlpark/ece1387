#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef PARALLEL
#include <pthread.h>
#endif

static unsigned long int iterations = 0;
static unsigned long int count = 0;

unsigned long int montecarlo(unsigned long int iterations)
{
    unsigned long int i, c = 0;
    double x, y, z;
    unsigned int seed = 1;
    
    for (i = 0; i < iterations; ++i) {
        x = (double)rand_r(&seed)/RAND_MAX;
        y = (double)rand_r(&seed)/RAND_MAX;
        z = x*x + y*y;
        if (z <= 1.0) {
            ++c;
        }
    }
    return c;
}

#ifdef PARALLEL
static unsigned int num_threads = 1;
// Insert your code below
static unsigned long int *rv_array;    
static unsigned long int *iter_array;

void *p_montecarlo(void * argument)
{
    unsigned int thread_num = (unsigned int)argument;
    unsigned long int i, c = 0;
    unsigned long int iter;
    double x, y, z;
    unsigned int seed = 1;

    iter = iter_array[thread_num];
//    printf("DEBUG thread num, count: %d, %lu\n", thread_num, iter);

    for (i = 0; i < iter; ++i) {
        x = (double)rand_r(&seed)/RAND_MAX;
        y = (double)rand_r(&seed)/RAND_MAX;
        z = x*x + y*y;
        if (z <= 1.0) {
            ++c;
        }
    }

    rv_array[thread_num] = c;
    return NULL;
}
#endif

int main(int argc, char *argv[])
{
    // Command line options for seed and iterations
    int c;
    #ifdef PARALLEL
    unsigned int i;
    unsigned long int iter;    
    unsigned long int remainder;    
    unsigned long int iter_per_thread;    
    
    while ((c = getopt (argc, argv, "i:t:")) != -1) {
    #else
    while ((c = getopt (argc, argv, "i:")) != -1) {
    #endif
        switch (c) {
        case 'i':
            iterations = strtoul(optarg, NULL, 10);
            break;
        #ifdef PARALLEL
        case 't':
           num_threads = strtoul(optarg, NULL, 10);
           if (num_threads == 0) {
               printf("%s: option requires an argument > 0 -- 't'\n", argv[0]);
                        return EXIT_FAILURE;
            }
           break;
        #endif
        default:
            return EXIT_FAILURE;
        }
    }
    // Check that iterations is present and valid
    if (iterations == 0) {
        printf("%s: option missing or requires an argument > 0 -- 'i'\n",
               argv[0]);
        return EXIT_FAILURE;
    }
    #ifdef PARALLEL
    rv_array = (unsigned long int*) malloc(num_threads*sizeof(unsigned long int));
    iter_array = (unsigned long int*) malloc(num_threads*sizeof(unsigned long int));

    // Insert your code below
    pthread_t thread[num_threads];
    iter_per_thread = iterations / num_threads;
    remainder = iterations % num_threads; 

    for (i = 0; i < num_threads; i++) {
      iter = (i == 0) ? remainder + iter_per_thread : iter_per_thread; 
      iter_array[i] = iter;
    }

    for (i = 0; i < num_threads; i++) {
      pthread_create(&thread[i], NULL, &p_montecarlo, (void *)i);
    }

    for (i = 0; i < num_threads; i++) {
      pthread_join(thread[i], NULL);
    }
    
    for (i = 0; i < num_threads; i++) {
       count += rv_array[i];
    }
    #else
    // Serial calculation of pi
    count += montecarlo(iterations);
    #endif

    double pi = (double)count/iterations*4;
    printf("%f\n", pi);

    return EXIT_SUCCESS;
}
