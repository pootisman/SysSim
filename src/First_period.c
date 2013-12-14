#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <memory.h>

#define VALID_ARGS "I:N:h"

#define STEPS 10000

#define HELP "Help\n-I <Classes file>\nClass file format:\n<# of device classes>\n<Class lambda>\t<Amount of devices in class>"

#define PARAL 1
#define SEQUE 2
#define STABLE 10000

double prevPcnt = 0.0;

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
      retVal += -(1.0/ *(pLambdas + i))*log((double)rand()/(double)RAND_MAX);
    }
  }

  return retVal;
}

void printPrcnt(double prcnt, double minDelta, unsigned int barlen){
  unsigned int i = 0;
  
  if(barlen == 0 || fabs(prcnt - prevPcnt) < minDelta){
    return;
  }
  
  prevPcnt = prcnt;
  
  (void)printf("[%3.1f%%][", prcnt);

  for(i = 0; i < barlen*(prcnt/100.0); i++){
    (void)printf("=");
  }

  for(;i < barlen; i++){
    (void)printf(" ");
  }

  (void)printf("]");

  (void)printf("\r");
  
  (void)fflush(stdout);
}

int main(int argc, char *argv[]){
  double *pDevTimes = NULL, *pLambdas = NULL, *pLVec = NULL, avgT = 0.0, stepper = 0.0, maxTime = 0.0;
  int i = 0;
  unsigned long int *pNDevs = NULL, *pNDevsTemp = NULL, nDevs = 0, j = 1, alive = 0, failFunc = 0, nClasses = 0, k = 0, rander = 0, nSteps = 0;
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

  pDevTimes = calloc(nDevs, sizeof(double));

  /* First period of life. */
  i = *pNDevs;

  for(j = 0, k = 0; j < nDevs; j++){
    
    if(j > i){
      i += *(pNDevs + k + 1);
      k++;
    }
    
    *(pDevTimes + j) = -(1.0/ *(pLambdas + k))*log((double)rand()/(double)RAND_MAX);
    avgT += *(pDevTimes + j);
    
    if(*(pDevTimes + j) > maxTime){
      maxTime = *(pDevTimes + j);
    }
    
    (void)printPrcnt((double)j/(double)nDevs*100.0, 0.1, 20);
  }
  
  avgT /= nDevs;

  (void)printf("Average operation time for first stage %f\n", avgT);

  histogram = fopen("histogram_furst.dat", "w");

  (void)fprintf(histogram, "#__Time______R(t)______Lambda(t)\n");

  nSteps = STEPS;

  /* Calculate R(t) and Lambda(t) */
  for(stepper = 0; stepper <= maxTime; stepper += maxTime/nSteps){
    failFunc = alive = 0;
    for(j = 0; j < nDevs; j++){
      if(*(pDevTimes + j) >= stepper){
	alive++;
	if(*(pDevTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    (void)printPrcnt((double)stepper/(double)maxTime*100.0, 0.1, 20);
    if(alive > STABLE){
      (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
    }
  }

  (void)fclose(histogram);

  /* ###################### */
  /* Second period of life. */
  /* ###################### */
  histogram = fopen("histogram_secund.dat", "w");

  (void)memcpy(pNDevsTemp, pNDevs, sizeof(unsigned long int)*nClasses);

  i = *pNDevs;

  for(j = 0, nDevs = 0; j < nClasses; j++){
    if(nDevs < *(pNDevs + j)){
      nDevs = *(pNDevs + j);
    }
  }
 
  i = 0;

  pLVec = calloc(nClasses, sizeof(double));

  avgT = maxTime = 0.0;

  /* Prepare bundles for simulation. */
  while(i < nDevs){
    k = 0;

    for(j = 0; j < nClasses; j++){
      if(*(pNDevsTemp + j) > 0){
	(*(pNDevsTemp + j))--;
	*(pLVec + k) = *(pLambdas + j);
	k++;
      }
    }

    *(pDevTimes + i) = generateTime(pLVec, k, SEQUE);
    avgT += *(pDevTimes + i);
    
    if(*(pDevTimes + i) > maxTime){
      maxTime = *(pDevTimes + i);
    }
 
    ++i;
    (void)printPrcnt((double)i/(double)nDevs*100.0, 0.1, 20);
  }

  /* Simulating  */
  avgT /= nDevs;

  (void)printf("Average operation time for second stage %f\n", avgT);

  (void)fprintf(histogram, "#__Time______R(t)______Lambda(t)\n");

  nSteps = STEPS;

  /* Calculate R(t) and Lambda(t) */
  for(stepper = maxTime/nSteps; stepper <= maxTime; stepper += maxTime/nSteps){
    failFunc = alive = 0;
    for(j = 0; j < nDevs; j++){
      if(*(pDevTimes + j) >= stepper){
	alive++;
	if(*(pDevTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    (void)printPrcnt((double)stepper/(double)maxTime*100.0, 0.1, 20);

    if(alive > STABLE){
      (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
    }
  }

  (void)fclose(histogram);

  /* ##################### */
  /* Third period of life. */
  /* ##################### */
  histogram = fopen("histogram_thurd.dat", "w");

  (void)memcpy(pNDevsTemp, pNDevs, sizeof(unsigned long int)*nClasses);

  i = *pNDevs;

  for(j = 0, nDevs = 0; j < nClasses; j++){
    if(nDevs < *(pNDevs + j)){
      nDevs = *(pNDevs + j);
    }
  }
 
  i = 0;

  avgT = maxTime = 0.0;

  /* Prepare bundles for simulation. */
  while(i < nDevs){
    k = 0;

    for(j = 0; j < nClasses; j++){
      if(*(pNDevsTemp + j) > 0){
	(*(pNDevsTemp + j))--;
	*(pLVec + k) = *(pLambdas + j);
	k++;
      }
    }

    *(pDevTimes + i) = generateTime(pLVec, k, PARAL);
    avgT += *(pDevTimes + i);
    
    if(*(pDevTimes + i) > maxTime){
      maxTime = *(pDevTimes + i);
    }
 
    ++i;
    (void)printPrcnt((double)i/(double)nDevs*100.0, 0.1, 20);
  }

  /* Simulating  */
  avgT /= nDevs;

  (void)printf("Average operation time for third stage %f\n", avgT);

  (void)fprintf(histogram, "#__Time______R(t)______Lambda(t)\n");

  nSteps = STEPS;

  /* Calculate R(t) and Lambda(t) */
  for(stepper = maxTime/nSteps; stepper <= maxTime; stepper += maxTime/nSteps){
    failFunc = alive = 0;
    for(j = 0; j < nDevs; j++){
      if(*(pDevTimes + j) >= stepper){
	alive++;
	if(*(pDevTimes + j) < (stepper + maxTime/nSteps)){
	  failFunc++;
	}
      }
    }
    (void)printPrcnt((double)stepper/(double)maxTime*100.0, 0.1, 20);

    if(alive > STABLE){
      (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
    }
  }

  (void)free(pLVec);
  (void)free(pDevTimes);
  (void)fclose(histogram);
  (void)free(pNDevs);
  (void)free(pLambdas);

  return EXIT_SUCCESS;
}
