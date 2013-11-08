#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>

#define VALID_ARGS "I:P:N:L:h"

#define STEPS 100

#define HELP "Help\nL <lambda>\nN <N of devices>"
/*
typedef struct task{
  unsigned int totalSteps ,nClasses, *pStepsEach;
  double *pLambdas, *stepArr, *Rt, *Lt;
}task;
*/
/* Random seed and its mutex. */
/*unsigned int cSeed = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
*/
/* Multithreaded modelling function.
void *tester(void *args){
  unsigned int i = 0, j = 0, k = 0, nSteps = 0;;
  double rGen 0.0, maxTime = 0.0;
  task *pTask = (task *)args;

  for(i = 0; i < pTask->totalSteps; i++){
    if(i <= *(pTask->pStepsEach + j)){
      j++;
    }
    
    (void)pthread_mutex_lock(&mutex);
    *(pTask->pVect + i) = -(1.0/ *(pTask->pLambdas + j))log((double)(RAND_MAX - rand_r(&cSeed))/(double)RAND_MAX);
    (void)pthread_mutex_unlock(&mutex);

    if(*(pTask->pVect + i) > maxTime){
      maxTime = *(pTask->pVect + i);
    }
  }

  nSteps = maxTime*10.0;
  k = 0;

  for(rGen = maxTime/nSteps; rGen < maxTime; rGen += maxTime/nSteps){
    alive = 0;
    failFunc = 0;
    for(j = 0; j < nDevs; j++){
      if(*(devTimes + j) >= stepper){
	alive++;
	if(*(devTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    *(pTask->stepArr + k) = rGen;
    *(pTask->Rt + k) = (double)alive/(double)(pTask->totalSteps);
    *(pTask->Lt + k) = (double)failFunc/(double)*((double)nSteps/(double)maxTime);
    ++k;
  }
}
*/

int main(int argc, char *argv[]){
  double *devTimes = NULL, *pLambdas = NULL, avgT = 0.0, stepper = 0.0, maxTime = 0.0, lambda = 0.0;
  int i = 0;
  unsigned int nDevs = 0, j = 1, l = 0, alive = 0, failFunc = 0, nClasses = 0, *pNDevs = NULL, k = 0, rander = 0, nSteps = 0/*, boost = 0*/;
  FILE *histogram = NULL, *classes = NULL, *randInit = NULL;

/*
  pthread_t *pThreads = NULL;
  pthread_attr_t threadAttr;
  task *parameters = NULL;
*/

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
	  pNDevs = calloc(nClasses, sizeof(unsigned int));

	  for(k = 0; k < nClasses; k++){
	    (void)fscanf(classes, "%lf\t%u\n", pLambdas + k, pNDevs + k);
	    nDevs += *(pNDevs + k);
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
/*
      case('B'):{
	boost = 1;
	nThreads = sysconf(_SC_NPROCESSORS_ONLN);
	pThreads = calloc(nThreads, sizeof(pthread_t));
	parameters = calloc(nThreads, sizeof(task));
	pthread_attr_init(&threadAttr);
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
	break;
      }
*/
      default:{
	(void)printf("Warning, unkown parameter met %c", i);
	break;
      }
    }
  }

  randInit = fopen("/dev/urandom", "rb");

  if(!randInit){
    (void)puts("No urandom device.");
  }else{
    (void)fread(&rander, 1 , sizeof(unsigned int), randInit);
    (void)srand(rander);
    (void)fclose(randInit);
  }

/*
  if(nClasses == 1 && !boost){
    pLambdas = calloc(1, sizeof(double));
    pNDevs = calloc(1, sizeof(unsigned int));
    *(pLambdas) = lambda;
    *(pNDevs) = nDevs;
  }
*/

#ifdef DEBUG
  (void)printf("Simulating %d devices.\n", nDevs);
  for(k = 0; k < nClasses; k++){
    (void)printf("Class %d ..[λ = %1.3f ,N = %d]\n", k, *(pLambdas + k), *(pNDevs + k));
  }
#endif

  devTimes = calloc(nDevs, sizeof(double));

  /* First period of life. */
  i = *pNDevs;

  for(j = 0, k = 0; j < nDevs; j++){
    if(j > i){
      i += *(pNDevs + k + 1);
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
/*}else{
    for(j = 0; j < nThreads; j++){
      nPlacesLeft = (parameters + j)->totalSteps = nDevs/nThreads;
      (parameters + j)->nClasses = nClasses;
      for(i = 0; i < nClasses && placesLeft > 0; i++){
	if(nPlacesLeft >= *(pNDevs + i)){
	  nPlacesLeft -= *(pNDevs + i);
	  nSims = *(pNDevs + i);
	}else{
	  nPlacesLeft = 0;
	  nSims = nDevs/nThreads;
	  *(pNDevs + i) -= nDevs/nThreads;
	}
	*((parameters + j)->(pStepsEach + i)) = nSims;
      }
      for(i = 0; i < nClasses; i++){
	*((parameters + j)->pLambdas + i) = *(pLambdas + i);
	*((parameters + j)->)
      }
    }
  }*/

  (void)printf("Average operation time for first stage %f\n", avgT);

  histogram = fopen("histogram_furst.dat", "w");

  (void)fprintf(histogram, "#__Time______R(t)______Lambda(t)\n");

  nSteps = STEPS;

  /* Calculate R(t) and Lambda(t) */
  for(stepper = maxTime/nSteps; stepper <= maxTime; stepper += maxTime/nSteps/10){
    alive = 0;
    failFunc = 0;
    for(j = 0; j < nDevs; j++){
      if(*(devTimes + j) >= stepper){
	alive++;
	if(*(devTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    if(alive > 500){
      (void)fprintf(histogram, "%f\t%f\t%f\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime));
    }
  }

  (void)fclose(histogram);
  maxTime = 0;
  /* Second period of life. */
  i = *pNDevs;

  for(j = 0, nDevs = 0; j < nClasses; j++){
    if(nDevs < *(pNDevs + j)){
      nDevs = *(pNDevs + j);
    }
  }
 
  (void)free(devTimes);
  devTimes = calloc(nDevs, sizeof(double));

  for(j = 0, k = 0; j < nDevs; j++){
    if(j > i){
      i += *(pNDevs + k + 1);
      k++;
    }
    lambda = 0;
    for(l = 0; l < nClasses; l++){
      if(*(pNDevs + l) > j){
        lambda += *(pLambdas + l);
      }
    }
    *(devTimes + j) = -(1.0/lambda)*log((double)(RAND_MAX - rand())/(double)RAND_MAX);
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

  histogram = fopen("histogram_secund.dat", "w");

  nSteps = ceil(maxTime)*10.0;

  /* Calculate R(t) and Lambda(t) */
  for(stepper = maxTime/nSteps; stepper < maxTime; stepper += maxTime/nSteps/10){
    alive = 0;
    failFunc = 0;
    for(j = 0; j < nDevs; j++){
      if(*(devTimes + j) >= stepper){
	alive++;
	if(*(devTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    if(alive > 500){
      (void)fprintf(histogram, "%f\t%f\t%f\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime));
    }
  }

  (void)fclose(histogram);
  (void)free(pNDevs);
  (void)free(pLambdas);
  (void)free(devTimes);

  return EXIT_SUCCESS;
}
