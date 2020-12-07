
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include "../test.h" 

#define NUMBER_THREADS 5 
#define RANDOM_MAX 50

//used to increment sums
#define INCREMENT(what, by_how_much) do {\
    uint64_t curr = what;\
    if (by_how_much < RANDOM_MAX / 2) sched_yield();\
    what = curr + by_how_much;\
} while(0)

/**
 * Aggregate sum calculated by all the threads
 */
//static uint64_t total_sum = 0;

//static var to hold mailbox id
static mailbox_id_t* this_id;


/**
 * Individual sums for each thread
 */
static uint64_t local_sums[NUMBER_THREADS];

//mutex lock used to exclude other threads
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * This is the code each of the threads will execute
 */
void* thread_function(void* arg)
{
	int thread_id = *(int*)arg;
    	local_sums[thread_id] = 0;
	
	uint64_t data = 0;
	uint64_t* readData = malloc(sizeof(uint64_t));
//	char name[5];
//	sprintf(name, "%s%d","mbox",thread_id);
	
    	
	for (size_t i = 0; i < 5; i++){
		data = (rand() % (RANDOM_MAX + 1));
		INCREMENT(local_sums[thread_id],data);
		pthread_mutex_lock(&lock);
		mbox_read_421_syscall(this_id,readData,sizeof(uint64_t),0);
		INCREMENT(*readData,data);
		mbox_write_421_syscall(this_id,readData,sizeof(uint64_t),0);
		//printf("\n%d::: data: %d, readData: %d\n",thread_id,data, *readData);
		pthread_mutex_unlock(&lock);
}


	free(readData);
    
    	printf("Thread %d finished and calculated: %ld\n", thread_id, local_sums[thread_id]);
  	return NULL;
}




int main(int argc, char** argv)
{
    time_t t;
    srand(time(&t));

    this_id = malloc(sizeof(mailbox_id_t));
    mbox_init_421_syscall();
    mbox_open_421_syscall("mbox", this_id, 10);
    
    //initialize value of mailbox
    uint64_t data = 0;    
    mbox_write_421_syscall(this_id,&data,sizeof(uint64_t),0);
    

    pthread_t thread_ids[NUMBER_THREADS];
    int thread_args[NUMBER_THREADS];

    // Create all the threads
    for (size_t i = 0; i < NUMBER_THREADS; i++)
    {
        thread_args[i] = i;
        // The thread starts working after this function call finishes
        pthread_create(&thread_ids[i], NULL, thread_function, &thread_args[i]);
    }
    
    // We wait for each of the threads to finish here
    for (size_t i = 0; i < NUMBER_THREADS; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

    // Manually sum the individual numbers. This is the correct total sum
    uint64_t real_sum = 0;
    for (size_t i = 0; i < NUMBER_THREADS; i++)
    {
    	    real_sum += (uint64_t)local_sums[i];
    }
	
    uint64_t* readData = malloc(sizeof(uint64_t));
    mbox_read_421_syscall(this_id,readData,sizeof(uint64_t),0); 
    
    // Print Color
    int ret;
    if (real_sum == *readData) 
    {
        // Will make the text green
        printf("\033[1;32m");
        ret = 0;
    }
    else
    {
        // Will make the text red
        printf("\033[1;31m");
        ret = 1;
    }

    printf("All threads are done. Total sum is %ld, expected %ld\n", *readData, real_sum);

    // Will reset the color
    printf("\033[0m");
	
    mbox_close_421_syscall(this_id);
    mbox_shutdown_421_syscall();
    free(this_id);
    free(readData);
    return ret;
}
