#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

typedef struct data
{
    int thread;
    int start;
    int count;
    char *directory;
    int numintegers;
} data;

void *CreateFiles(void *arguments)
{
    data *args = (data *)arguments;
    int numintegers = args->numintegers;
    int start = args->start;
    int count = args->count;
    int end = start + count;
    int thread = args->thread;
    int intArray[numintegers];
    unsigned int seed = clock();
    for (int i = start; i < end; i++)
    {
        char pathFile[100];
        sprintf(pathFile, "%s%s%d%s", args->directory, "/unsorted_", i, ".bin");

        printf("thread %d ** pathFile final :  %s\n", thread, pathFile);

        int filedescriptor = open(pathFile, O_WRONLY | O_CREAT, 0644);
        if (filedescriptor < 0)
        {
            perror("Error :");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < numintegers; j++)
        {
            int random = rand_r(&seed);
            intArray[j] = random;
            seed = random;
        }
        int sz = write(filedescriptor, intArray, sizeof(intArray));
        printf("%d size wriitten by thread %d ** pathFile:  %s\n", sz, thread, pathFile);
        if (close(filedescriptor) < 0)
        {
            perror("Error :");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "USAGE: %s <directory> <numfiles> <numintegers> <numthreads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *directory = argv[1];
    DIR *d;
    if ((d = opendir(directory)) != NULL)
    {
        closedir(d);
    }
    else
    {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
    int numfiles = atoi(argv[2]);
    if (numfiles <= 0)
    {
        fprintf(stderr, "ERROR: number of files must be a positive int (not '%s')\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    int numintegers = atoi(argv[3]);
    if (numintegers <= 0)
    {
        fprintf(stderr, "ERROR: number of integers must be a positive int (not '%s')\n", argv[3]);
        exit(EXIT_FAILURE);
    }
    int numthreads = atoi(argv[4]);
    if ((numthreads < 1) || (numthreads > numfiles))
    {
        fprintf(stderr, "ERROR: numthreads must be >= 1 and <= %d\n", numfiles);
        exit(EXIT_FAILURE);
    }

    data args[numthreads];
    pthread_t t[numthreads];
    int rc;
    int threadfilesCount = numfiles / numthreads;
    printf("threadfilesCount:  %d\n", threadfilesCount);
    int start_index = 0;
    /* start all of the threads */
    for (int i = 0; i < numthreads; i++)
    {
        args[i].start = start_index;
        int diff = numfiles - (start_index + threadfilesCount);
        int actualfilesCount = threadfilesCount;
        if (diff != 0 && (abs(diff) < threadfilesCount || (abs(diff) >= threadfilesCount && i == numthreads - 1)))
        {
            actualfilesCount += diff;
        }
        args[i].count = actualfilesCount;
        args[i].directory = directory;
        args[i].thread = i;
        args[i].numintegers = numintegers;
        rc = pthread_create(&t[i], NULL, CreateFiles, (void *)&args[i]);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
        start_index += actualfilesCount;
    }

    /* wait for threads to complete */
    for (int i = 0; i < numthreads; i++)
    {
        rc = pthread_join(t[i], NULL);
        if (rc != 0)
        {
            fprintf(stderr, "ERROR joining with thread %d (error==%d)\n", i, rc);
            exit(EXIT_FAILURE);
        }
    }
    return (0);
}