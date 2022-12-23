#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PING 0
#define PONG 1

volatile unsigned NumRounds = 0;
volatile unsigned PrevVal = PONG;

char *Message[2] = {"PING", "PONG"};

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

/*
 * pring the message, either "PING" or "PONG".
 * also checks to make sure that the correct message is being printed
 */
void pingpongprint(int thisval)
{
    /* if the threads are taking turns then PrevVal should always be different from thisval */
    pthread_mutex_lock(&lock);
    while (PrevVal == thisval)
    {
        // fprintf(stderr, "ERROR: received '%s' but expected '%s'\n", Message[thisval], Message[!thisval]);
        // exit(-1);
        pthread_cond_wait(&condition, &lock);
    }

    printf("%s\n", Message[thisval]);
    PrevVal = thisval;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&lock);
}

void *PingerPonger(void *tidptr)
{
    int tid = *((int *)tidptr);

    for (unsigned dex = 0; dex < NumRounds; dex++)
    {
        pingpongprint(tid);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "USAGE: %s <numrounds>\n", argv[0]);
        exit(-1);
    }

    NumRounds = atoi(argv[1]);
    if (NumRounds < 1)
    {
        fprintf(stderr, "ERROR: NumRounds must be >= 1\n");
        exit(-1);
    }

    int ping_tid = PING;
    int pong_tid = PONG;

    pthread_t pingthread;
    pthread_t pongthread;
    int rc;

    rc = pthread_create(&pingthread, NULL, PingerPonger, (void *)&ping_tid);
    if (rc)
    {
        fprintf(stderr, "ERROR; could not create PING thread. return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_create(&pongthread, NULL, PingerPonger, (void *)&pong_tid);
    if (rc)
    {
        fprintf(stderr, "ERROR; could not create PONG thread. return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_join(pingthread, NULL);
    if (rc != 0)
    {
        fprintf(stderr, "ERROR joining with PING (rc==%d)\n", rc);
        exit(-1);
    }

    rc = pthread_join(pongthread, NULL);
    if (rc != 0)
    {
        fprintf(stderr, "ERROR joining with PONG (rc==%d)\n", rc);
        exit(-1);
    }

    printf("SUCCESS!  (parent exiting)\n");
    return (0);
}