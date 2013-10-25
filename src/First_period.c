#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>

#define VALID_ARGS "N:L:h"

#define STEPS 300

#define HELP "Help\nL <lambda>\nN <N of devices>"

#define MAXTIME 1.0

int main(int argc, char *argv[]){
  double *devTimes = NULL, lambda = 0.0, avgT = 0.0, stepper = 0.0;
  int i = 0;
  unsigned int Ndevs = 0, j = 1, alive = 0;
  FILE *histogram = NULL;

  while((i = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(i){
      case('N'):{
	if(atoi(argv[j + 1]) > 0){
	  Ndevs = atoi(argv[j + 1]);
	  j += 2;
	  break;
	}else{
	  (void)puts("Invalid number of devices to test.");
	  return EXIT_FAILURE;
	}
      }
      case('L'):{
	if(atof(argv[j + 1]) > 0){
	  lambda = atof(argv[j + 1]);
	  j += 2;
	  break;
	}else{
	  (void)puts("Lambda can't be less than 0.");
	  return EXIT_FAILURE;
	}
      }
      case('h'):{
	(void)puts(HELP);
	return EXIT_SUCCESS;
      }
      default:{
	(void)printf("Warning, unkown parameter met %c", i);
	break;
      }
    }
  }

#ifdef DEBUG
  (void)printf("Simulating %d devices with %f labda.\n", Ndevs, lambda);
#endif

  devTimes = calloc(Ndevs, sizeof(double));

  for(j = 0; j < Ndevs; j++){
    *(devTimes + j) = -(1.0/lambda)*log((double)(RAND_MAX - rand())/(double)RAND_MAX);
    avgT += *(devTimes + j);
  }
  
  avgT /= Ndevs;

  (void)printf("Average operation time %f\n", avgT);

  histogram = fopen("histogram.dat", "w");

  /* Calculate R(t) */
  for(stepper = MAXTIME/STEPS; stepper < MAXTIME; stepper += MAXTIME/STEPS){
    alive = 0;
    for(j = 0; j < Ndevs; j++){
      if(*(devTimes + j) > stepper){
	alive++;
      }
    }
    (void)fprintf(histogram, "%f\t%f\t%f\n", stepper, (double)alive/(double)Ndevs, (double)alive/(double)Ndevs - exp(-lambda*stepper));
  }

  (void)fclose(histogram);

  (void)free(devTimes);

  return EXIT_SUCCESS;
}
