#include <stdio.h>
#include <pthread.h>
typedef struct
{
    int a;
    int b;
} myarg_t;

void *mythread(void *arg)
{
    myarg_t *args = (myarg_t *)arg;
    args->a = args->a + 1;
    args->b = args->b + 2;
    return (void *)args;
}

int main(int argc, char *argv[])
{
    pthread_t p;
    myarg_t *rvals;

    myarg_t args = {
        10,
        20};
    pthread_create(&p, NULL, mythread, &args);
    pthread_join(p, (void **)&rvals);
    printf("returned %d %d\n", rvals->a, rvals->b);
    return 0;
}