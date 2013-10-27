#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
/*#include <pthread.h>*/

#define VALID_ARGS "I:P:N:L:h"

#define STEPS 1000

#define HELP "Help\nL <lambda>\nN <N of devices>"
/*
typedef struct task{
  unsigned int totalSteps ,nModels, *pStepsEach;
  double *pLambda, *pVect;
}task;

void *tester(void *args){
  unsigned int i = 0, j = 0, k = 0;
  double rGen 0.0, maxTime = 0.0;
  task *pTask = (task *)args;

  for(i = 0; i < pTask->totalSteps; i++){
    if(i <= *(pTask->pStepsEach + j)){
      j++;
    }
    
    *(pTask->pVect + i) = -(1.0/ *(pTask->pLambda + j))log((double)(RAND_MAX - rand_r(cSeed))/(double)RAND_MAX);
    
    if(*(pTask->pVect + i) > maxTime){
      maxTime = *(pTask->pVect + i);
    }

    
  }
}
*/
int main(int argc, char *argv[]){
  double *devTimes = NULL, *pLambdas = NULL, avgT = 0.0, stepper = 0.0, maxTime = 0.0, lambda = 0.0;
  int i = 0;
  unsigned int nDevs = 0, j = 1, alive = 0, failFunc = 0, nClasses = 0, *pNSummons = NULL, k = 0;
  FILE *histogram = NULL, *classes = NULL;

  while((i = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(i){
      case('I'):{
	if(argv[j + 1]){
	  classes = fopen(argv[j + 1], "r");
	  if(!classes){
	    (void)puts("Unable to read class data, no file!");
	    return EXIT_FAILURE;
	  }
	  (void)fscanf(classes, "%u\n", &nClasses);
	  pLambdas = calloc(nClasses, sizeof(double));
	  pNSummons = calloc(nClasses, sizeof(unsigned int));

	  for(k = 0; k < nClasses; k++){
	    (void)fscanf(classes, "%lf\t%u\n", pLambdas + k, pNSummons + k);
	    nDevs += *(pNSummons + k);
	  }
	}else{
	  (void)puts("Argument error.");
	  return EXIT_FAILURE;
	}
	break;
      }
      case('N'):{
	if(atoi(argv[j + 1]) > 0 && nDevs == 0){
	  nDevs = atoi(argv[j + 1]);
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
/*      case('P'):{
	if(atoi(argv[j + 1]) > 0){
	  nPDevs = atoi(argv[j + 1]);
	  j += 2;
	  break;
	}else{
	  (void)puts("Can't put negative number of devs in parallel.");
	  return EXIT_FAILURE;
	}
      }*/
      default:{
	(void)printf("Warning, unkown parameter met %c", i);
	break;
      }
    }
  }

  if(nClasses == 1){
    pLambdas = calloc(1, sizeof(double));
    pNSummons = calloc(1, sizeof(unsigned int));
    *(pLambdas) = lambda;
    *(pNSummons) = nDevs;
  }

#ifdef DEBUG
  (void)printf("Simulating %d devices.\n", nDevs);
  for(k = 0; k < nClasses; k++){
    (void)printf("Class %d ..[λ = %1.3f ,N = %d]\n", k, *(pLambdas + k), *(pNSummons + k));
  }
#endif

  devTimes = calloc(nDevs, sizeof(double));

  i = *pNSummons;

  for(j = 0, k = 0; j < nDevs; j++){
    if(j > i){
      i += *(pNSummons + k + 1);
      k++;
    }
    *(devTimes + j) = -(1.0/ *(pLambdas + k))*log((double)(RAND_MAX - rand())/(double)RAND_MAX);
    avgT += *(devTimes + j);
#ifdef DEBUG
    (void)printf("%f\n", *(devTimes + j));
#endif
    if(*(devTimes + j) > maxTime){
      maxTime = *(devTimes + j);
    }
  }
  
  avgT /= nDevs;

  (void)printf("Average operation time for first stage %f\n", avgT);

  histogram = fopen("histogram_furst.dat", "w");

  /* Calculate R(t) and Lambda(t) */
  for(stepper = (double)maxTime/(double)STEPS; stepper < maxTime; stepper += maxTime/STEPS){
    alive = 0;
    failFunc = 0;
    for(j = 0; j < nDevs; j++){
      if(*(devTimes + j) > stepper){
	alive++;
	if(*(devTimes + j) < (stepper + maxTime/STEPS)){
	  failFunc++;
	}
      }
    }
    (void)fprintf(histogram, "%f\t%f\t%d\t%f\n", stepper, (double)alive/(double)nDevs, failFunc, (((double)failFunc)/(double)alive)*(double)STEPS/(double)maxTime);
  }

  (void)fclose(histogram);
  (void)free(pNSummons);
  (void)free(pLambdas);
  (void)free(devTimes);

  return EXIT_SUCCESS;
}
