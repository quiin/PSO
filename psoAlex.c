#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/* CUSTOM FILES */
#include "types.h"
#include "defines.h"
#include "functions.c"



///......................mode = 0 is min ................ mode = 1 is max

/* GLOBAL VARIABLES */
pthread_t* threads;
int sizeOfInt = sizeof(int);
int numThreads,antsCreated =0,antsTotal,threadsReady=0;
pthread_mutex_t creation_mutex = PTHREAD_MUTEX_INITIALIZER, mt = PTHREAD_MUTEX_INITIALIZER, vel_mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mt2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER, ready = PTHREAD_COND_INITIALIZER;
sem_t findGlobalBestMutex;
FILE* urandom;
BestGlobal* bestG;

/* FUNCTIONS */
void createThreads(int numThreads,int antsTotal,int tmpNumOfAnts, int function,int mode);
Ant* initAnt(int fun);
void* startPSO(void* tData);
void calVelocity(Ant** antP);
void moveAnt(Ant** antP, int fun, int mode);
void findGlobalBest(Ant* ant, int mode);


int main(void){

	urandom = fopen("/dev/urandom","r");
	if(urandom == NULL){
		printf("Can't open /dev/urandom !!!\n");
		exit(-1);
	}	
	
	int i, rc, tmpNumOfAnts, function, mode;

	bestG = NULL;	
	
	printf("Cuantas hormigas quieres tener?\n");
	scanf("%d", &antsTotal);		

	printf("***Funciones***\n");
	puts("1.- Sphere");
	puts("2.- Rosenbrock");
	puts("3.- Schwefel");
	puts("4.- Griewank");
	puts("5.- Rastrigin");

	printf("Que funcion quieres?\n");
	scanf("%d", &function);

	printf("Que modo quieres? (0 = min 1 = max)\n");
	scanf("%d", &mode);
		
	//init semaphore	
	sem_init(&findGlobalBestMutex,0,1);	

	tmpNumOfAnts = antsTotal % ANTS_PER_THREAD;	
	numThreads = tmpNumOfAnts == 0 ? antsTotal / ANTS_PER_THREAD : antsTotal / ANTS_PER_THREAD + 1;	
	
	createThreads(numThreads,antsTotal,tmpNumOfAnts,function,mode);	
	

	for(i = 0; i < numThreads; i++){
		rc = pthread_join(threads[i], NULL);
		if(rc){
			printf("Error joining thread %d pthread_join got %d\n", i, rc);
			exit(-1);
		}
	}
	printf("bestG value: %.2f at (%d,%d)\n", bestG->value,bestG->coos->x,bestG->coos->y);
	close(urandom);

	return 0;
}


void createThreads(int numThreads,int antsTotal,int tmpNumOfAnts, int function,int mode){

	threads = malloc(sizeof(pthread_t)*numThreads);

	int i,rc;

	TData* tData;
	
	for(i = 0; i < numThreads; i++){
		tData = (TData*)malloc(sizeof(TData));
		tData->id = i;
		tData->numAnts = antsTotal > tmpNumOfAnts ? ANTS_PER_THREAD : tmpNumOfAnts;		
		tData->function = function - 1;
		tData->mode = mode;		
		antsTotal -= ANTS_PER_THREAD;		
		rc = pthread_create(&threads[i], NULL, startPSO, (void*)tData);
		if(rc){
			printf("Error creating thread %d pthread_create got %d\n", i, rc);
			exit(-1);
		}
	}
}
Ant* initAnt(int fun){
	int randomNum;

	Ant* ant = (Ant*)malloc(sizeof(Ant));

	//position init
	ant->position = malloc(sizeof(Point));
	fread(&randomNum, sizeOfInt, 1, urandom);
	ant->position->x = randomNum % 500;
	fread(&randomNum, sizeOfInt, 1, urandom);
	ant->position->y = randomNum % 500;

	//velocity init
	ant->velocity = malloc(sizeof(Point));
	fread(&randomNum, sizeOfInt, 1, urandom);
	ant->velocity->x = abs(randomNum) % 2;
	fread(&randomNum, sizeOfInt, 1, urandom);
	ant->velocity->y = abs(randomNum) % 2;

	//bestPos init
	ant->bestPosition = malloc(sizeof(Point));	
	ant->bestPosition->x = ant->position->x;	
	ant->bestPosition->y = ant->position->y;

	ant->bestLocal = calFun(fun, ant);

	ant->cons1X = 2.0;
	ant->cons1X = 2.0;
	ant->cons1X = 2.0;
	ant->cons1X = 2.0;

	close(urandom);

	return ant;
}

void* startPSO(void* tData){
	int i, j;
	TData* myData = (TData*)tData;
	int id = myData->id;
	int numAnts = myData->numAnts;
	int function = myData->function;
	int mode = myData->mode;	

	//init swarm
	Ant* swarm[numAnts];
	/***START create ants and wait for all ants to be created***/
	
	//lock ant creation for other threads
	pthread_mutex_lock(&mt);
	for(i = 0; i < numAnts; i++){		
		swarm[i] = initAnt(function);
		antsCreated++;
	}

	//init bestG
	if(bestG==NULL){
		bestG = (BestGlobal*)malloc(sizeof(BestGlobal));	
		bestG->coos = (Point*)malloc(sizeof(Point));
		Ant* tmp = swarm[0];
		bestG->value = calFun(myData->function,tmp);
		bestG->coos->x = tmp->position->x;
		bestG->coos->y = tmp->position->y;
	}

	//unllock ant creation for other threads
	pthread_mutex_unlock(&mt);
	
	//wait for all ants to be created
	while(antsCreated<antsTotal){				
		pthread_cond_wait(&condition_var,&creation_mutex);
	}

	//all ants created -> wake all waiting threads
	if(antsCreated>=antsTotal-1){			
		pthread_cond_broadcast(&condition_var);
	}
	pthread_mutex_unlock(&creation_mutex);	
	
	/*****END create ants and wait for all ants to be created*****/

	//lock Global best access for other threads
	pthread_mutex_lock(&mt2);	
	for(i = 0; i < numAnts; i++){		
		sem_wait(&findGlobalBestMutex); //enter and block access		
		findGlobalBest(swarm[i], mode); //critical		
		sem_post(&findGlobalBestMutex); //leave and realese access		
	}
	threadsReady++;	
	pthread_mutex_unlock(&mt2);

	//wait for all threads to find global
	while(threadsReady<numThreads){	
		pthread_cond_wait(&ready,&vel_mutex);
	}

	//all threads are ready -> wake threads
	if(threadsReady>= numThreads){		
		pthread_cond_broadcast(&ready);		
	}
	pthread_mutex_unlock(&vel_mutex);
		
	for(i = 0; i < MAXITERATIONS; i++){		
		for(j = 0; j < numAnts; j++){			
			calVelocity(&swarm[j]);
			
			moveAnt(&(swarm[j]), function, mode);	
		}		
		for(j = 0; j < numAnts; j++){
			findGlobalBest(swarm[j], mode);
		}		
	}	
}

void calVelocity(Ant** antP){
	int randomNum;

	Ant* ant = *antP;

	long int localPart, globalPart;

	fread(&randomNum, sizeOfInt, 1, urandom);
	localPart = (long int)(ant->cons1X * (2 * (float)randomNum/(float)(RANDMAX)) * (ant->bestPosition->x - ant->position->x)) % RANDMAX;

	fread(&randomNum, sizeOfInt, 1, urandom);
	globalPart = (long int)(ant->cons2X * (2 * (float)randomNum/(float)(RANDMAX)) * (bestG->coos->x - ant->position->x)) % RANDMAX;

	ant->velocity->x = (int)(ant->velocity->x + localPart + globalPart) % 10;

	fread(&randomNum, sizeOfInt, 1, urandom);
	localPart = (long int)(ant->cons1Y * (2 * (float)randomNum/(float)(RANDMAX)) * (ant->bestPosition->y - ant->position->y)) % RANDMAX;

	fread(&randomNum, sizeOfInt, 1, urandom);
	globalPart = (long int)(ant->cons2Y * (2 * (float)randomNum/(float)(RANDMAX)) * (bestG->coos->y - ant->position->y)) % RANDMAX;

	ant->velocity->y = (int)(ant->velocity->y + localPart + globalPart) % 10;

	close(urandom);
}

void moveAnt(Ant** antP, int fun, int mode){			

	Ant* ant = *antP;

	if(ant->position->x + ant->velocity->x <= MAXRANGEX && ant->position->x + ant->velocity->x >= MINRANGEX)				
		ant->position->x += ant->velocity->x;		
		
	if(ant->position->y + ant->velocity->y <= MAXRANGEY && ant->position->y + ant->velocity->y >= MINRANGEY)		
		
		ant->position->y += ant->velocity->y;


	float newLocal = calFun(fun, ant);

	minOrMax(antP, fun, mode);

	if(ant->bestLocal - newLocal < bestG->value - newLocal){
		ant->cons1X -= .1;
		ant->cons1Y -= .1;
	}else{
		ant->cons2X -= .1;
		ant->cons2Y -= .1;
	}
}


void findGlobalBest(Ant* ant, int mode){
	if(mode == 0){
		
		if(ant->bestLocal < bestG->value){
			bestG->value = ant->bestLocal;
			bestG->coos->x = ant->bestPosition->x;
			bestG->coos->y = ant->bestPosition->y;
		}
	}else{
		if(ant->bestLocal > bestG->value){
			bestG->value = ant->bestLocal;
			bestG->coos->x = ant->bestPosition->x;
			bestG->coos->y = ant->bestPosition->y;
		}
	}	
}