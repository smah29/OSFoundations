`gcc -g -Wall -Werror pingpong2.c -o pingpong2 -lpthread`
- The two threads need to take turns, by signalling and waiting for the other threads to complete.
