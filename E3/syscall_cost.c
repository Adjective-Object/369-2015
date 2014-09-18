#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	int i;

	struct timeval start, end;
	double diff;
	int num_tries = 100;

	if(argc == 2) {
		num_tries = atoi(argv[1]);
	}

	/* MOST OF YOUR CODE WILL GO HERE */
	gettimeofday(&start, NULL);
	for (i; i<num_tries; i++){
		getpid();
	}
	gettimeofday(&end, NULL);
	/* TO HERE */

	diff = 1000000 * (end.tv_sec - start.tv_sec) +
			(end.tv_usec - start.tv_usec);
	diff = diff / num_tries;

    //EDIT: I changed the print statement because you probably
	// won't be measuring getpid()
	printf("Time is %f usec (num tries = %d)\n", diff, num_tries);
	// The number of times we need to execute the system call to get a
	// measurement that seems stable is:
	// NUM_TRIES: 1200
	return 0;
}
