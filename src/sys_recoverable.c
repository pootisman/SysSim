#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>

#define HELP "Help for recoverable system simulator.\n-I <System Config File>\n-h - Request help."
#define VALID_ARGS "C:I:N:D:h"

#define SINGLE 0
#define PARALLEL 1
#define SEQUENTIAL 2
#define REPLACING 3

#define DEAD 0x0F0F
#define ALIVE 0xF0F0
#define REPAIRING 0xFFFF
#define DISCRT 100.0
#define CEIL 50.0

double timer = 0.0;

typedef struct SUBSYS{
  struct SUBSYS **conts;
  double lambda, mu, t;
  unsigned short nSubs, bindType, state, nBrigsBusy, nPrim;
  int nBrigs;
}SUBSYS;

double getTime(SUBSYS *elem, int *nBrigs, unsigned short *nBrigsBusy){
  double temp1 = 0.0, temp2 = 0.0;
  unsigned short fail = 0x0, i = 0;

  if(!elem){
    (void)puts("Error, got null in getTime.");
    return -1.0;
  }

  if(!nBrigs){
    nBrigs = &(elem->nBrigs);
  }

  if(!nBrigsBusy){
    nBrigsBusy = &(elem->nBrigsBusy);
  }

  if(elem->bindType == SINGLE){
    if(elem->state == ALIVE && timer > elem->t){
      if(*(nBrigsBusy) < *(nBrigs)){
	temp1 = -(1.0/elem->mu)*log((double)rand()/(double)RAND_MAX);
	*(nBrigsBusy) += 1;
	elem->state = REPAIRING;
      }else{
        elem->state = DEAD;
      }
    }else if(elem->state == REPAIRING && timer > elem->t){
      temp1 = -(1.0/elem->lambda)*log((double)rand()/(double)RAND_MAX);
      elem->state = ALIVE;
      *(nBrigsBusy) -= 1;
    }
    elem->t += temp1;
  }else if(elem->bindType == SEQUENTIAL && timer > elem->t){
    temp1 = INFINITY;
    (void)puts("Seq sel");
    (void)fflush(stdout);
    for(i = 0; i < elem->nSubs; i++){
      if((temp2 = getTime(*elem->conts + i, nBrigs, nBrigsBusy)) < temp1){
        temp1 = temp2;
      }
      if((*elem->conts + i)->state == DEAD){
	elem->state = DEAD;
	fail = 0xFF;
	(void)puts("fsdfd");
      }
    }
    if(fail == 0x0){
      elem->state = ALIVE;
    }
  }
  
  return temp1;
}

inline unsigned char isAlive(SUBSYS *elem){
  if(!elem){
    (void)puts("Error, got NULL in isAlive.");
    return 0x0;
  }

 /* if(elem->bindType == SINGLE){(*/
    return (elem->state == ALIVE) ? (0xFF) : (0x0);
/*  }else{
    
  }
*/
  return 0x0;
}

SUBSYS **cpyDevs(const SUBSYS *orig, const unsigned int nTimes, const unsigned int nDevs){
  SUBSYS **newDevs = NULL;
  unsigned int i = 0, j = 0, k = 0;

  if(!orig || nTimes == 0){
    return NULL;
  }

  newDevs = calloc( nTimes, sizeof(SUBSYS *));

  for(i = 0; i < nTimes; i++){
    *(newDevs + i) = calloc(nDevs, sizeof(SUBSYS));
    for(j = 0; j < orig->nPrim; j++){
      (*(newDevs + i) + j)->lambda = (orig + j)->lambda;
      (*(newDevs + i) + j)->mu = (orig + j)->mu;
      (*(newDevs + i) + j)->t = (orig + j)->t;
      (*(newDevs + i) + j)->state = (orig + j)->state;
      (*(newDevs + i) + j)->nBrigs = (orig + j)->nBrigs;
      (*(newDevs + i) + j)->nBrigsBusy = (orig + j)->nBrigsBusy;
      (*(newDevs + i) + j)->bindType = (orig + j)->bindType;
      (*(newDevs + i) + j)->nSubs = (orig + j)->nSubs;
      (*(newDevs + i) + j)->conts = (orig + j)->conts;
    }
    for(;j < nDevs; j++){
      (*(newDevs + i) + j)->state = (orig + j)->state;
      (*(newDevs + i) + j)->bindType = (orig + j)->bindType;
      (*(newDevs + i) + j)->nSubs = (orig + j)->nSubs;
      (*(newDevs + i) + j)->conts = calloc((orig + j)->nSubs, sizeof(SUBSYS *));
      (*(newDevs + i) + j)->nBrigs = (orig + j)->nBrigs;
      (*(newDevs + i) + j)->nBrigsBusy = (orig + j)->nBrigsBusy;
      for(k = 0; k < (orig + j)->nSubs; k++){
        *((*(newDevs + i) + j)->conts + k) = *(newDevs + i) + ((*(orig + j)->conts + k) - orig);
      }
    }
  }

  return newDevs;

}

void freeDevs(SUBSYS **freeSys, unsigned int nDevs, unsigned int nDevsEach){
  unsigned int i = 0, j = 0;

  if(nDevs <= 0 || !freeSys){
    (void)puts("Error, failed to free models.");
    return;
  }

  for(i = 0; i < nDevs; i++){
    for(j = 0; j < nDevsEach; j++){
      if((*(freeSys + i) + j)->conts){
        (void)free((*(freeSys + i) + j)->conts);
      }
    }
    (void)free(*(freeSys + i));
  }
}

int main(int argc, char *argv[]){
  int i = 0;
  unsigned int nDevs = 0, nPrim = 0, init = 0, nBrigs = 0, nCopies = 0, nAlives = 0, j = 1, k = 0, l = 0;
  unsigned short m = 0;
  double ceiling = CEIL, discrete = DISCRT;
  FILE *fSysDesc = NULL, *initRand = NULL;
  SUBSYS *pSys = NULL, **pModels = NULL;
  
  while((i = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(i){
      case('I'):{
	if(argv[j + 1]){
	  fSysDesc = fopen(argv[j + 1], "r");
	  if(!fSysDesc){
	    (void)puts("Unable to read class data, no file!");
	    return EXIT_FAILURE;
	  }

	  if(fscanf(fSysDesc, "%u\t%u\t%u\n", &nDevs, &nPrim, &nBrigs) < 3){
	    (void)puts("Error reading number of devices.");
	    return EXIT_FAILURE;
	  }

	  pSys = calloc(nDevs, sizeof(SUBSYS));
	  pSys->nBrigs = nBrigs;
	  pSys->nPrim = nPrim;
	  for(k = 0; k < nDevs; k++){
	    (pSys + k)->state = ALIVE;
	    (pSys + k)->t = 0.0;
	    if(k < nPrim){
	      if(fscanf(fSysDesc, "%lf\t%lf\n", &(pSys + k)->lambda, &(pSys + k)->mu) < 2){
	        (void)puts("Error reading class data!");
	        return EXIT_FAILURE;
	      }
	      (pSys + k)->bindType = SINGLE;
	    }else{
	      if(fscanf(fSysDesc, "%hu\t%hu\t", &(pSys + k)->nSubs, &(pSys + k)->bindType) < 2){
	        (void)puts("Error reading subsystem data!");
	        return EXIT_FAILURE;
	      }
	      (pSys + k)->conts = calloc((pSys + k)->nSubs,sizeof(SUBSYS *));
	      for(l = 0; l < (pSys + k)->nSubs - 1; l++){
		if(fscanf( fSysDesc, "%hu\t", &m) < 1){
		  (void)puts("Error reading subsystems!");
		  return EXIT_FAILURE;
		}
		*((pSys + k)->conts + l) = pSys + m - 1;
	      }
	      if(fscanf( fSysDesc, "%hu\n", &m) < 1){
		(void)puts("Error reading subsystems!");
		return EXIT_FAILURE;
	      }
	      *((pSys + k)->conts + (pSys + k)->nSubs) = pSys + m - 1;
	    }
	  }
#ifdef DEBUG
	  (void)printf("#The system has %d devices and %d brigades.\n", nDevs, pSys->nBrigs);
	  for(k = 0; k < nDevs; k++){
	    (void)printf("#Device #%d:[%f, %d]\n", k, (pSys + k)->lambda, (pSys + k)->bindType);
	  }
#endif
	  j++;
	}else{
	  (void)puts("Argument error.");
	  return EXIT_FAILURE;
	}
	break;
      }
      case('C'):{
        if(argv[j + 1]){
	  ceiling = fabs(atof(argv[j + 1]));
	  j++;
	}
	break;
      }
      case('D'):{
        if(argv[j + 1]){
	  discrete = abs(atoi(argv[j + 1]));
	  j++;
	}
	break;
      }
      case('N'):{
        if(argv[j + 1]){
	  nCopies = abs(atoi(argv[j + 1]));
	  j++;
	}
	break;
      }
      case('h'):{
	(void)puts(HELP);
	return EXIT_SUCCESS;
      }
      default:{
	(void)printf("#Warning, unkown parameter met %c", i);
	break;
      }
    }
    j++;
  }

  initRand = fopen("/dev/urandom", "rb");

  pModels = cpyDevs(pSys, nCopies, nDevs);
#ifdef DEBUG
  (void)puts("#Multiplied devices, starting simulation.");
#endif
  if(!fread(&init, sizeof(unsigned int), 1, initRand)){
    (void)puts("#Warning random generator not initialized.");
  }

  (void)srand(init);

  while(timer < ceiling){
    nAlives = 0;
    for(j = 0; j < nCopies; j++){
      getTime(*(pModels + j), NULL, NULL);
      if(isAlive(*(pModels + j)) == 0xFF){
        nAlives++;
      }
    }
#ifdef DEBUG
    (void)printf("%f\t%f\n", timer ,(double)nAlives/nCopies);
#endif
    timer += 1.0/discrete;
  }

  (void)freeDevs(pModels, nCopies, nDevs);

  for(i = 0; i < nDevs;(void)free((pSys + i)->conts), i++);
 
  (void)free(pModels);
  (void)free(pSys);
  (void)fclose(fSysDesc);
  (void)fclose(initRand);

  return EXIT_SUCCESS;
}
