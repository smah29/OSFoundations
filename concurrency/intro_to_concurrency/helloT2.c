#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXTHREADS 50
#define SLEEPTIME 1

void *Factorial(void *tidptr)
{
  /* leave this sleep in place, it will help with debugging by varying the orders of the threads */
  sleep(SLEEPTIME);

  int tid = * ((int*) tidptr);

  if (tid > 20) {
    fprintf(stderr, "ERROR: thread %d exit to avoid long long overflow\n", tid);
    return NULL;
  }
  
  unsigned long long factorial = 1;
  for (int j = 2; j <= tid; j++) {
    factorial *= j;
  }

  printf("Thread %d result is %llu\n", tid, factorial);
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "USAGE: helloT <nthreads>\n");
    exit(-1);
  }

  int nthreads = atoi(argv[1]);
  if (nthreads < 1) {
    fprintf(stderr, "ERROR: numthreads must be >= 1\n");
    exit(-1);
  }

  printf("User requested %d threads\n", nthreads);

  /* just create one thread for now */
  /* CS532 students will improve it to support up to MAXTHREADS threads */

  if(nthreads > MAXTHREADS) nthreads = MAXTHREADS; 
  pthread_t t[nthreads];
  int rc;
  int arr[nthreads];
  for(int i = 0 ; i < nthreads ; i++){
	arr[i] = i + 1;
        rc = pthread_create(&t[i], NULL, Factorial, (void *) &arr[i]);
	if (rc){
   		fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
    		exit(-1);
  	}
  }
  for(int i = 0 ; i < nthreads ; i++){
        rc = pthread_join(t[i], NULL);
        if (rc){
                fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }
  }
  exit(0);
}
