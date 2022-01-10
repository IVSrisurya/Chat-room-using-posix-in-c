/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define NUMBER_OF_THREADS 3

// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

struct Qnode {
    task data;
    struct Qnode *next;
};

struct Queue{
    struct Qnode *front, *rear;
};

struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

task worktodo;
pthread_mutex_t mutexQ;
pthread_cond_t condQ;
sem_t sem;
pthread_t bee[NUMBER_OF_THREADS];
struct Queue *queue;


// initialize the thread pool
void pool_init(void)
{
    pthread_mutex_init(&mutexQ,NULL);
    pthread_cond_init(&condQ,NULL);
    sem_init(&sem, 0, 0);
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
        pthread_create(&bee[i], NULL, worker, NULL);
    queue = malloc(sizeof(struct Queue));
}

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
    pthread_mutex_lock(&mutexQ);
    struct Qnode *temp = malloc(sizeof(struct Qnode));
    temp->data = t;
    temp->next = NULL;
    
	if (queue->front == NULL)
		queue->front = queue->rear = temp;
	else
	{
		queue->rear->next = temp;
		queue->rear = temp;
	}
    pthread_cond_signal(&condQ);
	pthread_mutex_unlock(&mutexQ);
    
    return 0;
}

// remove a task from the queue
task dequeue() 
{
    pthread_mutex_lock(&mutexQ);
    while(queue->front == NULL)
        pthread_cond_wait(&condQ,&mutexQ);
    
    task worktodo = queue->front->data;
	queue->front = queue->front->next;

    if(queue->front == NULL)
        queue->rear = NULL;
    pthread_mutex_unlock(&mutexQ);
    
    return worktodo;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    while(1) {
        task temp;
        sem_wait(&sem);
        temp = dequeue();
        execute(temp.function, temp.data);

    }
    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    printf("\nThread %lu is occupied\n", pthread_self());
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    worktodo.function = somefunction;
    worktodo.data = p;

    enqueue(worktodo);
    sem_post(&sem);

    return 0;
}

// shutdown the thread pool
void pool_shutdown(void)
{
    sem_destroy(&sem);
    pthread_mutex_destroy(&mutexQ);
    pthread_cond_destroy(&condQ);
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
        pthread_cancel(bee[i]);
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
        pthread_join(bee[i], NULL);
}