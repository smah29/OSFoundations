#include <stdio.h>  // stderr
#include <stdlib.h> // EXIT_FAILURE
#include <dirent.h> // DIR
#include <fcntl.h>  // O_RDONLY
#include <sys/stat.h>
#include <string.h>   // strcmp
#include <unistd.h>   // close(filedescriptor)
#include <sys/mman.h> //PROT_READ
#include <pthread.h>

typedef struct data
{
    int thread;
    int start;
    int count;
    char *directory;
} data;

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void selectionSort(int *arr, int n)
{
    int i, j, min_idx;

    for (i = 0; i < n - 1; i++)
    {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;
        swap(&arr[min_idx], &arr[i]);
    }
}

void *CheckFiles(void *arguments)
{
    data *args = (data *)arguments;
    int start = args->start;
    int count = args->count;
    int end = start + count;
    int thread = args->thread;
    char *directory = args->directory;
    printf("thread : %d, first index : %d, last index : %d\n", thread, start, end - 1);
    for (int i = start; i < end; i++)
    {

        char oldFileName[30];
        sprintf(oldFileName, "%s%d%s", "unsorted_", i, ".bin");
        // start of unsorted file section
        char unsortedFile[100];
        sprintf(unsortedFile, "%s%s%s", directory, "/", oldFileName);
        printf("thread %d unsortedFile %s\n", thread, unsortedFile);
        /* Open the file for reading. */
        int unsorted_fd = open(unsortedFile, O_RDONLY);
        if (unsorted_fd < 0)
        {
            fprintf(stderr, "Error in opening file %s:\n", unsortedFile);
            exit(EXIT_FAILURE);
        }
        struct stat unsorted_stats;
        /* Get the size of the file. */
        if (stat(unsortedFile, &unsorted_stats) < 0)
        {
            fprintf(stderr, "Error in fetching meta data of file %s:\n", unsortedFile);
            exit(EXIT_FAILURE);
        }
        int *unsorted_ptr = mmap(0, unsorted_stats.st_size,
                                 PROT_READ, MAP_PRIVATE,
                                 unsorted_fd, 0);
        if (unsorted_ptr == MAP_FAILED)
        {
            fprintf(stderr, "Mapping Failed for file %s\n", unsortedFile);
            exit(EXIT_FAILURE);
        }
        if (close(unsorted_fd) < 0)
        {
            fprintf(stderr, "Error in closing file %s:\n", unsortedFile);
            exit(EXIT_FAILURE);
        }
        /* SIZE OF ARRAY */
        int unsorted_arr_size = unsorted_stats.st_size / sizeof(int);
        int intArray[unsorted_arr_size];
        for (int i = 0; i < unsorted_arr_size; i++)
        {
            intArray[i] = unsorted_ptr[i];
        }
        if (munmap(unsorted_ptr, unsorted_stats.st_size) != 0)
        {
            fprintf(stderr, "UnMapping Failed for file %s\n", unsortedFile);
            exit(EXIT_FAILURE);
        }
        // end of unsorted file section
        // start of sorted file section
        char *newFileName;
        newFileName = strstr(oldFileName, "sorted");
        char sortedFile[100];
        sprintf(sortedFile, "%s%s%s", directory, "/sorted/", newFileName);
        printf("thread %d sortedFile %s\n", thread, sortedFile);
        /* Open the file for reading. */
        int sorted_fd = open(sortedFile, O_RDONLY);
        if (sorted_fd < 0)
        {
            fprintf(stderr, "Error in opening file %s:\n", sortedFile);
            exit(EXIT_FAILURE);
        }
        struct stat sorted_stats;
        if (stat(sortedFile, &sorted_stats) < 0)
        {
            fprintf(stderr, "Error in fetching meta data of file %s:\n", sortedFile);
            exit(EXIT_FAILURE);
        }
        /* SIZE OF ARRAY */
        int sorted_arr_size = sorted_stats.st_size / sizeof(int);
        if (sorted_arr_size != unsorted_arr_size)
        {
            fprintf(stderr, "Error : file %s size=%d is different from file %s size=%d\n", sortedFile, sorted_arr_size, unsortedFile, unsorted_arr_size);
            exit(EXIT_FAILURE);
        }
        int *sorted_ptr = mmap(0, sorted_stats.st_size,
                               PROT_READ, MAP_PRIVATE,
                               sorted_fd, 0);
        if (sorted_ptr == MAP_FAILED)
        {
            fprintf(stderr, "Mapping Failed for file %s\n", sortedFile);
            exit(EXIT_FAILURE);
        }
        if (close(sorted_fd) < 0)
        {
            fprintf(stderr, "Error in closing file %s:\n", sortedFile);
            exit(EXIT_FAILURE);
        }
        int sortedArray[sorted_arr_size];
        for (int i = 0; i < sorted_arr_size; i++)
        {
            sortedArray[i] = sorted_ptr[i];
        }
        if (munmap(sorted_ptr, sorted_stats.st_size) != 0)
        {
            fprintf(stderr, "UnMapping Failed for file %s\n", sortedFile);
            exit(EXIT_FAILURE);
        }
        // end of sorted file section
        selectionSort(intArray, unsorted_arr_size);
        // intArray is now sorted
        for (int i = 0; i < sorted_arr_size; i++)
        {
            // compare each element of both arrays
            if (!(intArray[i] == sortedArray[i]))
            {
                fprintf(stderr, "\n**Error** : %s is not the proper sort of %s\n\n", sortedFile, unsortedFile);
                exit(EXIT_FAILURE);
                break;
            }
        }
        printf("\nthread %d **Success** : %s is the proper sort of %s\n\n", thread, sortedFile, unsortedFile);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "USAGE: %s <directory> <numthreads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *directory = argv[1];
    DIR *d;
    if ((d = opendir(directory)) == NULL)
    {
        fprintf(stderr, "%s directory doesn't exists :\n", directory);
        exit(EXIT_FAILURE);
    }
    int max = -1;
    struct dirent *dent;
    while ((dent = readdir(d)) != NULL)
    {
        char *fileName = dent->d_name;
        // skipping below cases
        if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0 || (*fileName) == '.' || strcmp(fileName, "sorted") == 0)
        {
            continue;
        }
        int i = atoi(&fileName[9]); // fetching the i from unsorted_i.bin
        if (i > max)
        {
            max = i;
        }
    }
    closedir(d);
    int numfiles = max + 1;
    printf("numfiles of files in directory %s is %d\n", directory, numfiles);
    if (numfiles == 0)
    {
        fprintf(stderr, "ERROR: Empty directory %s\n", directory);
        exit(EXIT_FAILURE);
    }
    char sortedDir[100];
    sprintf(sortedDir, "%s%s", directory, "/sorted/");
    DIR *sorted_d;
    if ((sorted_d = opendir(sortedDir)) == NULL)
    {
        fprintf(stderr, "%s directory doesn't exists :\n", sortedDir);
        exit(EXIT_FAILURE);
    }
    struct dirent *sorted_dent;
    int numSortedFiles = 0;
    while ((sorted_dent = readdir(sorted_d)) != NULL)
    {
        char *sortedFileName = sorted_dent->d_name;
        // skipping below cases
        if (strcmp(sortedFileName, ".") == 0 || strcmp(sortedFileName, "..") == 0 || (*sortedFileName) == '.')
        {
            continue;
        }
        numSortedFiles++;
    }
    closedir(sorted_d);
    if (numSortedFiles == 0)
    {
        fprintf(stderr, "ERROR: Empty directory %s\n", sortedDir);
        exit(EXIT_FAILURE);
    }
    int numthreads = atoi(argv[2]);
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
        rc = pthread_create(&t[i], NULL, CheckFiles, (void *)&args[i]);
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