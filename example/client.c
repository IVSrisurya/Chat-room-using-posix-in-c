/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp = (struct data*)param;

    printf("Adding two values %d and %d, the result is %d\n",temp->a, temp->b, temp->a + temp->b);
}

void sub(void *param)
{
    struct data *temp = (struct data*)param;

    printf("Subtracting two values %d and %d, the result is %d\n",temp->a, temp->b, temp->a - temp->b);
}

void mult(void *param)
{
    struct data *temp = (struct data*)param;

    printf("Multiplying two values %d and %d, the result is %d\n",temp->a, temp->b, temp->a * temp->b);
}

void div(void *param)
{
    struct data *temp = (struct data*)param;

    printf("Dividing two values %d and %d, the result is %f\n",temp->a, temp->b, (double)temp->a / (double)temp->b);
}



int main(void)
{
    // create some work to do
    struct data work1;
    struct data work2;
    struct data work3;
    struct data work4;

    printf("Enter 2 values to add: ");
    scanf("%d", &work1.a);
    scanf("%d", &work1.b);

    printf("Enter 2 values to subtract: ");
    scanf("%d", &work2.a);
    scanf("%d", &work2.b);

    printf("Enter 2 values to multiply: ");
    scanf("%d", &work3.a);
    scanf("%d", &work3.b);

    printf("Enter 2 values to divide: ");
    scanf("%d", &work4.a);
    scanf("%d", &work4.b);

    printf("\n");

    // initialize the thread pool
    pool_init();

    // submit the work to the queue
    pool_submit(&add,&work1);
    pool_submit(&sub,&work2);
    pool_submit(&mult,&work3);
    pool_submit(&div,&work4);


    sleep(1);
    pool_shutdown();

    return 0;
}
