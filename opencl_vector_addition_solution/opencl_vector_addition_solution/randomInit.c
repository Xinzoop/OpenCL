// Allocates a matrix with random float entries.
#include <stdio.h>
#include <stdlib.h>

#define RANGE_MAX 100

int randomInit()
{
//srandom(clock());
float   dataf = (float) random()/RAND_MAX*RANGE_MAX;
int data= (int) dataf;
return(data);
}

/*
void main(){
//srandom(1);
printf("Result=%d RAND_MAX=%ld\n", randomInit(),RAND_MAX);
printf("Result=%d RAND_MAX=%ld\n", randomInit(),RAND_MAX);
printf("Result=%d RAND_MAX=%ld\n", randomInit(),RAND_MAX);
printf("Result=%d RAND_MAX=%ld\n", randomInit(),RAND_MAX);
printf("Result=%d RAND_MAX=%ld\n", randomInit(),RAND_MAX);
printf("Result=%d RAND_MAX=%ld\n", randomInit(),RAND_MAX);
}
*/
