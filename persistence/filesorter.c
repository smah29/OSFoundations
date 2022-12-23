#include <stdio.h>  // stderr
#include <stdlib.h> // EXIT_FAILURE
#include <dirent.h> // DIR
#include <string.h> // strcmp
#include <fcntl.h>  // O_RDONLY
#include <unistd.h> // close(filedescriptor)
#include <sys/stat.h>

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// for each index find the min index swap the min index data with the given index and move ahead
void selectionSort(int arr[], int n)
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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "USAGE: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *directory = argv[1];
    DIR *d;
    struct dirent *dent;
    struct stat unsorted_stats;

    if ((d = opendir(directory)) != NULL)
    {
        while ((dent = readdir(d)) != NULL)
        {
            char *oldFileName = dent->d_name;
            if (!(strcmp(oldFileName, ".") == 0 || strcmp(oldFileName, "..") == 0 || (*oldFileName) == '.' || strcmp(oldFileName, "sorted") == 0))
            {
                char unsortedFile[100];
                sprintf(unsortedFile, "%s%s%s", directory, "/", oldFileName);
                printf("unsortedFile %s\n", unsortedFile);
                int unsorted_fd = open(unsortedFile, O_RDONLY);
                if (unsorted_fd < 0)
                {
                    perror("Error :");
                    exit(EXIT_FAILURE);
                }
                if (stat(unsortedFile, &unsorted_stats) == 0)
                {
                    char sortedFile[100];
                    char sortedDir[100];
                    sprintf(sortedDir, "%s%s", directory, "/sorted/");
                    struct stat sorted_stats;
                    if (!(stat(sortedDir, &sorted_stats) == 0 && S_ISDIR(sorted_stats.st_mode)))
                    {
                        int check = mkdir(sortedDir, 0744);
                        if (!check)
                            printf("Sorted Directory created\n");
                        else
                        {
                            fprintf(stderr, "Unable to create Sorted directory\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    char *newFileName;
                    newFileName = strstr(oldFileName, "sorted");
                    sprintf(sortedFile, "%s%s%s", directory, "/sorted/", newFileName);
                    printf("sortedFile %s\n", sortedFile);
                    int sorted_fd = open(sortedFile, O_WRONLY | O_CREAT, 0644);
                    if (sorted_fd < 0)
                    {
                        perror("Error :");
                        exit(EXIT_FAILURE);
                    }
                    int intArray[unsorted_stats.st_size / sizeof(int)];
                    read(unsorted_fd, intArray, sizeof(intArray));
                    int n = sizeof(intArray) / sizeof(intArray[0]);
                    selectionSort(intArray, n);
                    write(sorted_fd, intArray, sizeof(intArray));
                    if (close(sorted_fd) < 0)
                    {
                        perror("Error :");
                        exit(EXIT_FAILURE);
                    }
                }
                if (close(unsorted_fd) < 0)
                {
                    perror("Error :");
                    exit(EXIT_FAILURE);
                }
            }
        }
        closedir(d);
    }
    else
    {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
}
