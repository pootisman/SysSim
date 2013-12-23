#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <memory.h>

#ifdef THREADS
#include <unistd.h>
#include <pthread.h>
#endif

#define VALID_ARGS "R:I:N:h"

#define STEPS 10000

#define HELP "Help\n-I <Classes file>\nClass file format:\n<# of device classes>\n<Class lambda>\t<Amount of devices in class>"

#define PARAL 1
#define SEQUE 2
#define STABLE 10000

#ifdef THREADS
typedef struct THRD_ARG{
  unsigned long int startIndx, nDevs, stopIndx;
  double stepper, stopTime, step;
  double *pFailFuncs, *pAlives;
  double *pDevTimes;
}THRD_ARG;
#endif

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

#ifdef THREADS
void *threadedMonitor(void *args){
  THRD_ARG *pArgs = (THRD_ARG *)args;
  unsigned long int i = 0, j = 0;

  if(!pArgs || !pArgs->pDevTimes || !pArgs->pFailFuncs || !pArgs->pAlives){
    (void)puts("Error, got NULL in threadedMonitor.");
    (void)pthread_exit(NULL);
  }
  
  for(i = pArgs->startIndx; pArgs->stepper < pArgs->stopTime && i < pArgs->stopIndx; pArgs->stepper += pArgs->step, i++){
    *(pArgs->pFailFuncs + i) = *(pArgs->pAlives + i) = 0;
    for(j = 0; j < pArgs->nDevs; j++){
      if(*(pArgs->pDevTimes + j) >= pArgs->stepper){
	*(pArgs->pAlives + i)+=1.0;
	if(*(pArgs->pDevTimes + j) < (pArgs->stepper + pArgs->step)){
	  *(pArgs->pFailFuncs + i)+=1.0;
	}
      }
    }
  }
  (void)pthread_exit(NULL);
}
#endif

inline void remap(double prevMax, double newMax, double *pAlive, double *pAliveNew, double *pFailFunc, double *pFailFuncNew, unsigned int nSteps){
  unsigned int i = 0, j = 0;

  if(!pAlive || !pFailFunc){
    (void)puts("Error, got NULL while remapping.");
    return;
  }

  for(i = 0; i < nSteps; i++){
    for(j = 0; j < nSteps; j++){
      if(prevMax/nSteps*j >= newMax/nSteps*i && prevMax/nSteps*j < newMax/nSteps*i){
        *(pFailFuncNew + i) += *(pFailFunc + j);
	*(pAliveNew + i) += *(pAlive + j);
      }
    }
  }
}

int main(int argc, char *argv[]){
  double *pDevTimes = NULL, *pLambdas = NULL, *pLVec = NULL, avgT = 0.0, maxTime = 0.0;
  int i = 0;
  unsigned long int *pNDevs = NULL, *pNDevsTemp = NULL, nDevs = 0, j = 1, nClasses = 0, k = 0, rander = 0, nSteps = 0, nDevs1 = 0, nDevs2 = 0, nDevs3 = 0;
  FILE *histogram = NULL, *classes = NULL, *randInit = NULL;
  
#ifdef THREADS
  double *pAlives = NULL, *pFailFuncs = NULL, *pAlivesA[3] = {NULL}, *pFailFuncA[3] = {NULL}, maxTime1 = 0.0, prevMax1 = 0.0, maxTime2 = 0.0, maxTime3 = 0.0;
  unsigned long int nThreads = 1, l = 0, nRestarts = 0;
  THRD_ARG *pThrdParams = NULL;
  pthread_t *pThreads = NULL;
  pthread_attr_t threadAttr;
#else
  unsigned long int failFunc = 0, alive = 0;
  double stepper = 0.0;
#endif
  
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
	  j++;
	  (void)fclose(classes);
	}else{
	  (void)puts("Argument error.");
	  return EXIT_FAILURE;
	}
	break;
      }
      case('R'):{
#ifdef THREADS
	if(argv[j + 1]){
	  nRestarts = atoi(argv[j + 1]);
	}
	j++;
#else
	(void)puts("Error, only in threaded program.");
#endif
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
    j++;
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

#ifdef THREADS
  nThreads = sysconf(_SC_NPROCESSORS_ONLN);
  pThrdParams = calloc(nThreads, sizeof(THRD_ARG));
  pThreads = calloc(nThreads, sizeof(pthread_t));
#endif
  
#ifdef DEBUG
  (void)printf("Simulating %lu devices.\n", nDevs);
  for(k = 0; k < nClasses; k++){
    (void)printf("Class %lu ..[λ = %1.3f ,N = %lu]\n", k, *(pLambdas + k), *(pNDevs + k));
  }
#ifdef THREADS
  (void)printf("Running with %ld threads.\n", nThreads);
#endif
#endif

  pDevTimes = calloc(nDevs, sizeof(double));
  nSteps = STEPS;

#ifdef THREADS
  for(k = 0; k < 3; k++){
    pAlivesA[k] = calloc(nSteps, sizeof(double));
    pFailFuncA[k] = calloc(nSteps, sizeof(double));
  }

  nDevs1 = nDevs;

  for(l = 0; l < nRestarts; l++){
    maxTime = maxTime1 = maxTime2 = maxTime3 = avgT = 0;
#endif
/* First period of life. */
    i = *pNDevs;

    for(j = 0, k = 0; j < nDevs1; j++){
    
      if(j > i){
        i += *(pNDevs + k + 1);
        k++;
      }
    
      *(pDevTimes + j) = -(1.0/ *(pLambdas + k))*log((double)rand()/(double)RAND_MAX);
      if(*(pDevTimes + j) < INFINITY){
        avgT += *(pDevTimes + j);
      }

      if(*(pDevTimes + j) > maxTime){
        maxTime = *(pDevTimes + j);
      }
    
      (void)printPrcnt((double)j/(double)nDevs1*100.0, 0.1, 20);
    }
  
    avgT /= nDevs1;

    (void)printf("Average operation time for first stage %f\n", avgT);

    nSteps = STEPS;

/* Calculate R(t) and Lambda(t) */
#ifndef THREADS
    for(stepper = 0; stepper <= maxTime; stepper += maxTime/nSteps){
      failFunc = alive = 0;
      for(j = 0; j < nDevs1; j++){
        if(*(pDevTimes + j) >= stepper){
	  alive++;
  	  if(*(pDevTimes + j) < (stepper + maxTime/nSteps)){
	    failFunc++;
  	  }
        }
      }
      (void)printPrcnt((double)stepper/(double)maxTime*100.0, 0.1, 20);
      if(alive > STABLE){
        (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs1, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
      }
    }
#else
    prevMax1 = maxTime1;
    maxTime1 = maxTime;
    pAlives = calloc(nSteps, sizeof(double));
    pFailFuncs = calloc(nSteps, sizeof(double));
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    if(l){
      remap(prevMax1, maxTime1, pAlivesA[0], pAlivesA[0], pFailFuncA[0], pFailFuncA[0], nSteps);
    }

    for(i = 0; i < nThreads; i++){
      (pThrdParams + i)->stopIndx = nSteps/nThreads * (i+1);
      (pThrdParams + i)->startIndx = nSteps/nThreads * i;
      (pThrdParams + i)->stepper = maxTime1/nThreads * i;
      (pThrdParams + i)->nDevs = nDevs1;
      (pThrdParams + i)->pDevTimes = pDevTimes;
      (pThrdParams + i)->pAlives = pAlives;
      (pThrdParams + i)->pFailFuncs = pFailFuncs;
      (pThrdParams + i)->stopTime = maxTime1/nThreads * (i+1);
      (pThrdParams + i)->step = maxTime1/nSteps;
    }
 
    for(i = 0; i < nThreads; i++){
      pthread_create((pThreads + i), &threadAttr, threadedMonitor, (void *)(pThrdParams + i));
    }
 
    pthread_attr_destroy(&threadAttr);
  
    for(i = 0; i < nThreads; i++){
      pthread_join(*(pThreads + i), NULL);
    }
  
    for(i = 0; i < nSteps && *(pAlives + i) > STABLE; i++){
      *(pAlivesA[0] + i) += *(pAlives + i);
      *(pFailFuncA[0] + i) += *(pFailFuncs + i);
    }
  
    (void)free(pAlives);
    (void)free(pFailFuncs);

#endif

/* ###################### */
/* Second period of life. */
/* ###################### */

    (void)memcpy(pNDevsTemp, pNDevs, sizeof(unsigned long int)*nClasses);

    i = *pNDevs;

    for(j = 0, nDevs2 = 0; j < nClasses; j++){
      if(nDevs2 < *(pNDevs + j)){
        nDevs2 = *(pNDevs + j);
      }
    }

    i = 0;

    pLVec = calloc(nClasses, sizeof(double));

    avgT = maxTime = 0.0;

/* Prepare bundles for simulation. */
    while(i < nDevs2){
      k = 0;

      for(j = 0; j < nClasses; j++){
        if(*(pNDevsTemp + j) > 0){
	  (*(pNDevsTemp + j))--;
	  *(pLVec + k) = *(pLambdas + j);
	  k++;
        }
      }

      *(pDevTimes + i) = generateTime(pLVec, k, SEQUE);
      if(*(pDevTimes + i) < INFINITY){
        avgT += *(pDevTimes + i);
      }
    
      if(*(pDevTimes + i) > maxTime){
        maxTime = *(pDevTimes + i);
      }
 
      ++i;
      (void)printPrcnt((double)i/(double)nDevs2*100.0, 0.1, 20);
    }

/* Simulating  */
    avgT /= nDevs2;

    (void)printf("Average operation time for second stage %f\n", avgT);

    nSteps = STEPS;

#ifndef THREADS
/* Calculate R(t) and Lambda(t) */
    for(stepper = maxTime/nSteps; stepper <= maxTime; stepper += maxTime/nSteps){
      failFunc = alive = 0;
      for(j = 0; j < nDevs2; j++){
        if(*(pDevTimes + j) >= stepper){
	  alive++;
	  if(*(pDevTimes + j) < (stepper + maxTime/nSteps)){
	    failFunc++;
	  }
        }
      }
      (void)printPrcnt((double)stepper/(double)maxTime*100.0, 0.1, 20);

      if(alive > STABLE){
        (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs2, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
      }
    }
#else
    maxTime2 = maxTime;
    pAlives = calloc(nSteps, sizeof(double));
    pFailFuncs = calloc(nSteps, sizeof(double));
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
  
    for(i = 0; i < nThreads; i++){
      (pThrdParams + i)->stopIndx = nSteps/nThreads * (i+1);
      (pThrdParams + i)->startIndx = nSteps/nThreads * i;
      (pThrdParams + i)->stepper = maxTime2/nThreads * i;
      (pThrdParams + i)->nDevs = nDevs2;
      (pThrdParams + i)->pDevTimes = pDevTimes;
      (pThrdParams + i)->pAlives = pAlives;
      (pThrdParams + i)->pFailFuncs = pFailFuncs;
      (pThrdParams + i)->stopTime = maxTime2/nThreads * (i+1);
      (pThrdParams + i)->step = maxTime2/nSteps;
    }
  
    for(i = 0; i < nThreads; i++){
      pthread_create((pThreads + i), &threadAttr, threadedMonitor, (void *)(pThrdParams + i));
    }  

    pthread_attr_destroy(&threadAttr);
  
    for(i = 0; i < nThreads; i++){
      pthread_join(*(pThreads+i), NULL);
    }
  
    for(i = 0; i < nSteps && *(pAlives + i) > STABLE; i++){
      *(pAlivesA[1] + i) += *(pAlives + i);
      *(pFailFuncA[1] + i) += *(pFailFuncs + i);
    }
  
    (void)free(pAlives);
    (void)free(pFailFuncs);
#endif

/* ##################### */
/* Third period of life. */
/* ##################### */

    (void)memcpy(pNDevsTemp, pNDevs, sizeof(unsigned long int)*nClasses);

    i = *pNDevs;

    for(j = 0, nDevs3 = 0; j < nClasses; j++){
      if(nDevs3 < *(pNDevs + j)){
        nDevs3 = *(pNDevs + j);
      }
    }
 
    i = 0;

    avgT = maxTime = 0.0;

/* Prepare bundles for simulation. */
    while(i < nDevs3){
      k = 0;

      for(j = 0; j < nClasses; j++){
        if(*(pNDevsTemp + j) > 0){
  	  (*(pNDevsTemp + j))--;
	  *(pLVec + k) = *(pLambdas + j);
	  k++;
        }
      }

      *(pDevTimes + i) = generateTime(pLVec, k, PARAL);
      
      if(*(pDevTimes + i) < INFINITY){
        avgT += *(pDevTimes + i);
      }

      if(*(pDevTimes + i) > maxTime){
        maxTime = *(pDevTimes + i);
      }
 
      ++i;
      (void)printPrcnt((double)i/(double)nDevs3*100.0, 0.1, 20);
    }

/* Simulating  */
    avgT /= nDevs3;

    (void)printf("Average operation time for third stage %f\n", avgT);

    nSteps = STEPS;

#ifndef THREADS
/* Calculate R(t) and Lambda(t) */
    for(stepper = maxTime/nSteps; stepper <= maxTime; stepper += maxTime/nSteps){
      failFunc = alive = 0;
      for(j = 0; j < nDevs3; j++){
        if(*(pDevTimes + j) >= stepper){
	  alive++;
	  if(*(pDevTimes + j) < (stepper + maxTime/nSteps)){
	    failFunc++;
	  }
        }
      }
      (void)printPrcnt((double)stepper/(double)maxTime*100.0, 0.1, 20);

      if(alive > STABLE){
        (void)fprintf(histogram, "%f\t%f\t%f\t%lu\n", stepper, (double)alive/(double)nDevs3, ((double)failFunc/(double)alive)*((double)nSteps/(double)maxTime), alive);
      }
    }
#else
    maxTime3 = maxTime;
    pAlives = calloc(nSteps, sizeof(double));
    pFailFuncs = calloc(nSteps, sizeof(double));
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
  
    for(i = 0; i < nThreads; i++){
      (pThrdParams + i)->stopIndx = nSteps/nThreads * (i+1);
      (pThrdParams + i)->startIndx = nSteps/nThreads * i;
      (pThrdParams + i)->stepper = maxTime3/nThreads * i;
      (pThrdParams + i)->nDevs = nDevs3;
      (pThrdParams + i)->pDevTimes = pDevTimes;
      (pThrdParams + i)->pAlives = pAlives;
      (pThrdParams + i)->pFailFuncs = pFailFuncs;
      (pThrdParams + i)->stopTime = maxTime3/nThreads * (i+1);
      (pThrdParams + i)->step = maxTime3/nSteps;
    }
  
    for(i = 0; i < nThreads; i++){
      pthread_create((pThreads + i), &threadAttr, threadedMonitor, (void *)(pThrdParams + i));
    }
  
    pthread_attr_destroy(&threadAttr);
  
    for(i = 0; i < nThreads; i++){
      pthread_join(*(pThreads+i), NULL);
    }
 
    for(i = 0; i < nSteps && *(pAlives + i) > STABLE; i++){
      *(pAlivesA[2] + i) += *(pAlives + i);
      *(pFailFuncA[2] + i) += *(pFailFuncs + i);
    }
    
    (void)free(pAlives);
    (void)free(pFailFuncs);
  }
 
  histogram = fopen("histogram_furst.dat", "w");

  for(i = 0; i < nSteps; i++){
    (void)fprintf(histogram, "%f\t%f\t%f\t%f\n", maxTime1/nSteps*i, (double)*(pAlivesA[0] + i)/nRestarts/(double)nDevs1, ((double)*(pFailFuncA[0] + i)/nRestarts/(double)*(pAlivesA[0] + i)/nRestarts)*((double)nSteps/(double)maxTime1), *(pAlivesA[0] + i)/nRestarts);
  }
  
  (void)fclose(histogram);
  histogram = fopen("histogram_secund.dat", "w");

  for(i = 0; i < nSteps; i++){
    (void)fprintf(histogram, "%f\t%f\t%f\t%f\n", maxTime2/nSteps*i, (double)*(pAlivesA[1] + i)/nRestarts/(double)nDevs2, ((double)*(pFailFuncA[1] + i)/nRestarts/(double)*(pAlivesA[1] + i)/nRestarts)*((double)nSteps/(double)maxTime2), *(pAlivesA[1] + i)/nRestarts);
  }

  (void)fclose(histogram);
  histogram = fopen("histogram_thurd.dat", "w");

  for(i = 0; i < nSteps; i++){
    (void)fprintf(histogram, "%f\t%f\t%f\t%f\n", maxTime3/nSteps*i, (double)*(pAlivesA[2] + i)/nRestarts/(double)nDevs3, ((double)*(pFailFuncA[2] + i)/nRestarts/(double)*(pAlivesA[2] + i)/nRestarts)*((double)nSteps/(double)maxTime3), *(pAlivesA[2] + i)/nRestarts);
  }

  (void)free(pThreads);
  (void)free(pThrdParams);
  (void)free(pAlivesA[0]);
  (void)free(pFailFuncA[0]);
  (void)free(pAlivesA[1]);
  (void)free(pFailFuncA[1]);
  (void)free(pAlivesA[2]);
  (void)free(pFailFuncA[2]);
#endif
  (void)free(pLVec);
  (void)free(pDevTimes);
  (void)fclose(histogram);
  (void)free(pNDevs);
  (void)free(pNDevsTemp);
  (void)free(pLambdas);

  return EXIT_SUCCESS;
}
