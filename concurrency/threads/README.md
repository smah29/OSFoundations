- sum.c: repeatedly sum numbers using multiple threads
- compute the value of numthreads * (sum of all integers from 0 to Round) 
- for every value of Round from 1 to numrounds the program also uses a closed-form solution to check the result after each round. if it finds an error, then it exits.


- gcc -Wall -Werror -lpthread sum2.c -o sum2
- ./sum2 4 500
- gcc -Wall -Werror -lpthread sum3.c -o sum3
- ./sum3 4 500
- 4 threads and 500 is Rounds
