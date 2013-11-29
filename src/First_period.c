#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <memory.h>

#define VALID_ARGS "I:P:N:h"

#define STEPS 100

#define HELP "Help\nL <lambda>\nN <N of devices>"

#define PARAL 1
#define SEQUE 2

/*
typedef struct deviceBundle{
  unsigned char nDevs, linkType;
  double *pDevTime;
}deviceBundle;
*/

inline double generateTime(double *pLambdas, unsigned char nClasses, unsigned char connection){
  double retVal = 0.0, tmp1 = 0.0;
  unsigned char i = 0;

  if(!pLambdas){
    (void)puts("Error, got NULL for lambdas.");
    return -1.0;
  }

  tmp1 = retVal = -(1.0/ *pLambdas)*log((double)rand()/(double)RAND_MAX);

  if(connection == SEQUE){
    for(i = 1; i < nClasses; i++){
      tmp1 = -(1.0/ *(pLambdas + i))*log((double)rand()/(double)RAND_MAX);
      if(tmp1 < retVal){
	retVal = tmp1;
      }
    }
  }else if(connection == PARAL){
    for(i = 1; i < nClasses; i++){
      tmp1 = -(1.0/ *(pLambdas + i))*log((double)rand()/(double)RAND_MAX);
      if(tmp1 > retVal){
        retVal = tmp1;
      }
    }
  }

  return retVal;
}

/*
char ded(deviceBundle *pBndl, double testTime){
  unsigned int i = 0;

  switch(pBndl->linkType){
    case(1):{
      for(i = 0; i < pBndl->nDevs; i++){
	if(*(pBndl->pDevTime + i) > testTime){
	  return 32;
	}
      }
      return -32;
    }
    case(2):{
      for(i = 0; i < pBndl->nDevs; i++){
	if(*(pBndl->pDevTime + i) < testTime){
	  return -32;
	}
      }
      return 32;
    }
    default:{
      (void)puts("Unknown mode detected.");
      return 0;
    }
  }
}
*/

int main(int argc, char *argv[]){
  double *devTimes = NULL, *pLambdas = NULL, avgT = 0.0, stepper = 0.0, maxTime = 0.0;
  deviceBundle *pDevBundl = NULL;
  int i = 0;
  unsigned long int nDevs = 0, j = 1, alive = 0, failFunc = 0, nClasses = 0, *pNDevs = NULL, k = 0, rander = 0, nSteps = 0, *pNDevsTemp = NULL;
  FILE *histogram = NULL, *classes = NULL, *randInit = NULL;

  while((i = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(i){
      case('I'):{
	if(argv[j + 1]){
	  classes = fopen(argv[j + 1], "r");
	  if(!classes){
	    (void)puts("Unable to read class data, no file!");
	    return EXIT_FAILURE;
	  }
	  if(!fscanf(classes, "%lu\n", &nClasses)){
	    (void)puts("Error reading class data!");
	    return EXIT_FAILURE;
	  }

	  pLambdas = calloc(nClasses, sizeof(double));
	  pNDevs = calloc(nClasses, sizeof(unsigned long int));
	  pNDevsTemp = calloc(nClasses, sizeof(unsigned long int));

	  for(k = 0; k < nClasses; k++){
	    if(!fscanf(classes, "%lf\t%lu\n", pLambdas + k, pNDevs + k)){
	      (void)puts("Error reading class data!");
	      return EXIT_FAILURE;
	    }
	    nDevs += *(pNDevs + k);
	  }

	  (void)memcpy(pNDevsTemp, pNDevs, nClasses*sizeof(unsigned long int));
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

  randInit = fopen("/dev/urandom", "rb");

  if(!randInit){
    (void)puts("No urandom device.");
  }else{
    if(fread(&rander, 1 , sizeof(unsigned long int), randInit)){
      (void)srand(rander);
    }
    (void)fclose(randInit);
  }

#ifdef DEBUG
  (void)printf("Simulating %lu devices.\n", nDevs);
  for(k = 0; k < nClasses; k++){
    (void)printf("Class %lu ..[λ = %1.3f ,N = %lu]\n", k, *(pLambdas + k), *(pNDevs + k));
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
    *(devTimes + j) = -(1.0/ *(pLambdas + k))*log((double)rand()/(double)RAND_MAX);
    avgT += *(devTimes + j);
/*#ifdef DEBUG
    (void)printf("%f\n", *(devTimes + j));
#endif*/
    if(*(devTimes + j) > maxTime){
      maxTime = *(devTimes + j);
    }
  }
  
  avgT /= nDevs;

  (void)printf("Average operation time for first stage %f\n", avgT);

  histogram = fopen("histogram_furst.dat", "w");

  (void)fprintf(histogram, "#__Time______R(t)______Lambda(t)\n");

  nSteps = STEPS;

  /* Calculate R(t) and Lambda(t) */
  for(stepper = maxTime/nSteps; stepper <= maxTime; stepper += maxTime/nSteps/10){
    failFunc = alive = 0;
    for(j = 0; j < nDevs; j++){
      if(*(devTimes + j) >= stepper){
	alive++;
	if(*(devTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    if(alive > 500){
      (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
    }
  }

  (void)fclose(histogram);

  /* ###################### */
  /* Second period of life. */
  /* ###################### */
  histogram = fopen("histogram_secund.dat", "w");

  i = *pNDevs;

  for(j = 0, nDevs = 0; j < nClasses; j++){
    if(nDevs < *(pNDevs + j)){
      nDevs = *(pNDevs + j);
    }
  }
 
  (void)free(devTimes);
  pDevBundl = calloc( nDevs, sizeof(deviceBundle));

  i = 0;

  /* Prepare bundles for simulation. */
  while(i < nDevs){
    for(j = 0; j < nClasses; j++){
      if(*(pNDevsTemp + j + k) > 0){
	(*(pNDevsTemp + j + k))--;
	(pDevBundl + i)->nDevs++;
	k = 0;
      }else{
	j--;
	k++;
      }
    }

    (pDevBundl + i)->pDevTime = calloc((pDevBundl + i)->nDevs, sizeof(double));
    (void)memcpy(pNDevsTemp, pNDevs, nClasses*sizeof(unsigned long int));

    for(j = 0; j < (pDevBundl + i)->nDevs; j++){
	*((pDevBundl + i)->pDevTime + j) =  -(1.0/ *(pLambdas + j))*log((double)(RAND_MAX - rand())/(double)RAND_MAX);
    }

    (pDevBundl + i)->linkType = SEQUE;

    ++i;
  }

  /* Simulating  */
  for(stepper = maxTime/STEPS; stepper <= maxTime*4; stepper += maxTime/STEPS){

    alive = failFunc = 0;

    for(i = 0; i < nDevs; i++){
      if(!ded(pDevBundl + i, stepper)){
	alive++;
	if(ded(pDevBundl + i, stepper + maxTime/STEPS)){
	  failFunc++;
	}
      }
    }
    if(alive > 500){
      (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
    }
  }

  for(i = 0; i < nDevs; i++){
    (void)free((pDevBundl + i)->pDevTime);
  }

  (void)free(pDevBundl);
  (void)fclose(histogram);
  (void)free(pNDevs);
  (void)free(pLambdas);

  return EXIT_SUCCESS;
}
