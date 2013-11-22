#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>

#define VALID_ARGS "I:P:N:L:h"

#define STEPS 100

#define HELP "Help\nL <lambda>\nN <N of devices>"

#define PARAL 1
#define SEQUE 2

typedef struct deviceBundle{
  unsigned char nDevs, linkType;
  double *pDevLambdas;
}

int main(int argc, char *argv[]){
  double *devTimes = NULL, *pLambdas = NULL, avgT = 0.0, stepper = 0.0, maxTime = 0.0, lambda = 0.0;
  deviceBundle *pDevBundl = NULL;
  int i = 0;
  unsigned long int nDevs = 0, j = 1, l = 0, alive = 0, failFunc = 0, nClasses = 0, *pNDevs = NULL, k = 0, rander = 0, nSteps = 0, *pNDevsTemp = NULL;
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
	  (void)fscanf(classes, "%Lu\n", &nClasses);
	  pLambdas = calloc(nClasses, sizeof(double));
	  pNDevs = calloc(nClasses, sizeof(unsigned long int));

	  for(k = 0; k < nClasses; k++){
	    (void)fscanf(classes, "%lf\t%Lu\n", pLambdas + k, pNDevs + k);
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

  randInit = fopen("/dev/urandom", "rb");

  if(!randInit){
    (void)puts("No urandom device.");
  }else{
    (void)fread(&rander, 1 , sizeof(unsigned long int), randInit);
    (void)srand(rander);
    (void)fclose(randInit);
  }

#ifdef DEBUG
  (void)printf("Simulating %Lu devices.\n", nDevs);
  for(k = 0; k < nClasses; k++){
    (void)printf("Class %Lu ..[λ = %1.3f ,N = %Lu]\n", k, *(pLambdas + k), *(pNDevs + k));
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
      (void)fprintf(histogram, "%f\t%f\t%f\t%Lu\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
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
  pDevBundl = calloc( nDevs, sizeof(deviceBundle));

  i = 0;

  while(i < nDevs){
    for(j = 0; j < nClasses; j++){
      if(*(pNDevsTemp + j) > 0){
	*(pNDevsTemp + j)--;
	(pDevBundl + i)->nDevs++;
      }else{
	j--;
	k++;
      }
    }

    (pDevBundl + i)->pDevLambdas = calloc((pDevBundl + i)->nDevs, sizeof(double));
    (void)memcpy(pNDevsTemp, pNDevs, nClasses*sizeof(unsigned long int));

    for(j = 0; j < (pDevBundl + i)->nDevs; j++){
	(pDevBundl + i)->(pDevLambdas + j) =  -(1.0/ *(pLambdas + j))*log((double)(RAND_MAX - rand())/(double)RAND_MAX);
    }

    ++i;
  }

  (void)fclose(histogram);
  (void)free(pNDevs);
  (void)free(pLambdas);
  (void)free(devTimes);

  return EXIT_SUCCESS;
}
