#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXDIMENSION 2
#define RASTRIGIN_CONS 10
#define PI 3.141592
#define MAXITERATIONS 1
#define MINCONSTANT 0.5
#define MAXRANGEX 500
#define MAXRANGEY 500
#define MINRANGEX -500
#define MINRANGEY -500
#define RANDMAX 4294967296
#define ANTS_PER_THREAD 50

///......................mode = 0 is min ................ mode = 1 is max
pthread_t* threads;
int sizeOfInt = sizeof(int);
int numThreads,antsCreated =0,antsTotal;
pthread_mutex_t creation_mutex = PTHREAD_MUTEX_INITIALIZER, mt = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

FILE* urandom;

typedef struct point{
	int x;
	int y;
} Point;

typedef struct thread_data{
	int id;
	int numAnts;
	int function;
	int mode;
} TData;

typedef struct bestGlobal{
	Point* coos;
	float value;
} BestGlobal;

BestGlobal* bestG;

typedef struct ant{	
	Point* position;	
	Point* velocity;
	Point* bestPosition;
	float bestLocal;	
	float cons1X;
	float cons2X;
	float cons1Y;
	float cons2Y;
}Ant;

int getCooDim(int dimension, Ant* ant){
	switch (dimension){
		case 0: return ant->position->x;
		case 1: return ant->position->y;
		// case 2; return ant->position->z;
		// case 3; return ant->position->k;
		default: return -1;
	}
}

float sphere(Ant* ant){
	int i, coo;
	float result = 0.0;
	for(i = 0; i < MAXDIMENSION; i++){
		coo = getCooDim(i, ant); 	
		result += pow((float)coo, 2.0);
	}
	return result;
}

float rosenbrock(Ant* ant){
	int i, coo;
	float result = 0.0;
	for(i = 0; i < MAXDIMENSION - 1; i++){	
		coo = getCooDim(i, ant); 	
		result += 100 * ( getCooDim(i + 1, ant) - pow((float)coo, 2.0)) + pow((float)(coo - 1), 2.0);
	}
	return result;
}

float schwefel(Ant* ant){
	int i, coo;
	float result = 418.9829 * MAXDIMENSION;
	for(i = 0; i < MAXDIMENSION; i++){	
		coo = getCooDim(i, ant); 	
		result -= coo * sin(sqrt(abs(coo)));
	}
	return result;
}

float griewank(Ant* ant){
	int i, coo;
	float result = 0.0, resMult = 1.0;
	for(i = 0; i < MAXDIMENSION; i++){	
		coo = getCooDim(i, ant); 	
		result += pow(coo, 2.0);
		resMult *= cos((float)coo / sqrt(i + 1));
	}
	result /= 4000;
	return result - resMult + 1;	
}

float rastrigin (Ant* ant){
	int i, result = RASTRIGIN_CONS, coo;
	for(i = 0; i < MAXDIMENSION; i++){	
		coo = getCooDim(i, ant); 	
		result += pow(coo, 2.0) - RASTRIGIN_CONS * cos(2 * PI * coo);
	}
	return result;
}



float calFun(int fun, Ant* ant){
	//printf("calFun\n");
	switch(fun){
		case 0: 		
			return sphere(ant);
		case 1: 
			return rosenbrock(ant);
		case 2: 
			return schwefel(ant);
		case 3: 
			return griewank(ant);
		case 4: 
			return rastrigin(ant);
	}
}


Ant* initAnt(int fun){
	int randomNum;

	//Randomizer
	// FILE* urandom = fopen("/dev/urandom","r");
	// if(urandom == NULL){
	// 	printf("Can't open /dev/urandom !!!\n");
	// 	exit(-1);
	// }		

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

Point* calVelocity(Ant* ant){
	int randomNum;
	long int localPart, globalPart;

	//Randomizer
	// FILE* urandom = fopen("/dev/urandom","r");
	// if(urandom == NULL){
	// 	printf("Can't open /dev/urandom !!!\n");
	// 	exit(-1);
	// }		

	Point* vel = (Point*)malloc(sizeof(Point));

	fread(&randomNum, sizeOfInt, 1, urandom);
	localPart = (long int)(ant->cons1X * (2 * (float)randomNum/(float)(RANDMAX)) * (ant->bestPosition->x - ant->position->x)) % RANDMAX;

	fread(&randomNum, sizeOfInt, 1, urandom);
	globalPart = (long int)(ant->cons2X * (2 * (float)randomNum/(float)(RANDMAX)) * (bestG->coos->x - ant->position->x)) % RANDMAX;

	vel->x = (int)(ant->velocity->x + localPart + globalPart) % 10;

	fread(&randomNum, sizeOfInt, 1, urandom);
	localPart = (long int)(ant->cons1Y * (2 * (float)randomNum/(float)(RANDMAX)) * (ant->bestPosition->y - ant->position->y)) % RANDMAX;

	fread(&randomNum, sizeOfInt, 1, urandom);
	globalPart = (long int)(ant->cons2Y * (2 * (float)randomNum/(float)(RANDMAX)) * (bestG->coos->y - ant->position->y)) % RANDMAX;

	vel->y = (int)(ant->velocity->y + localPart + globalPart) % 10;

	//printf("vel->x=%d vel->y=%d\n", vel->x, vel->y);

	close(urandom);

	return vel;
}

void minOrMax(Ant** antP, int fun, int mode){

	Ant* ant = *antP;
	
	float newLocal = calFun(fun, ant);	

	if(mode == 0){
		if(ant->bestLocal > newLocal){
			ant->bestLocal = newLocal;
			ant->bestPosition->x = ant->position->x;
			ant->bestPosition->y = ant->position->y;
		}else{
			ant->position->x = ant->bestPosition->x;
			ant->position->y = ant->bestPosition->y;
		}
	}else{
		if(ant->bestLocal < newLocal){
			ant->bestLocal = newLocal;
			ant->bestPosition->x = ant->position->x;
			ant->bestPosition->y = ant->position->y;
		}else{
			ant->position->x = ant->bestPosition->x;
			ant->position->y = ant->bestPosition->y;
		}
	}	
}

void moveAnt(Point* vel, Ant** antP, int fun, int mode){			

	Ant* ant = *antP;

	if(ant->position->x + vel->x <= MAXRANGEX && ant->position->x + vel->x >= MINRANGEX)		
		// printf("LAST %d + %d\n", ant->position->x, vel->x);
		ant->position->x += vel->x;
		// printf("NEW %d\n", ant->position->x);
		
	if(ant->position->y + vel->y <= MAXRANGEY && ant->position->y + vel->y >= MINRANGEY)		
		//printf("%d + %d\n", ant->position->y, vel->y);
		ant->position->y += vel->y;
		//printf("%d\n", ant->position->y);
		
	
	//printf("x=%d y=%d\n", ant->position->x,ant->position->y);

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
		//printf("%f  < %f\n", ant->bestLocal, bestG->value);
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
	//printf("%f\n", bestG->value);
}

void* startPSO(void* tData){
	int i, j;
	TData* myData = (TData*)tData;
	int id = myData->id;
	int numAnts = myData->numAnts;
	int function = myData->function;
	int mode = myData->mode;	
	
	Point* vel;

	//init swarm with ants
	Ant* swarm[numAnts];
	pthread_mutex_lock(&mt);
	for(i = 0; i < numAnts; i++){
		// printf("Thread %d created ant %d of %d\n", id, i + 1, numAnts);
		swarm[i] = initAnt(function);
		antsCreated++;
	}
	pthread_mutex_unlock(&mt);

	//THREAD HAS TO WAIT FOR OTHER THREADS CREATING THEIR ANTS
	//WE NEED TO SELECT GLOBALBEST THEN
	//wait until all ants are created
	while(antsCreated<antsTotal){
		printf("Thread %d is waiting. Current ants: %d/%d\n", myData->id,antsCreated,antsTotal);
		pthread_cond_wait(&condition_var,&creation_mutex);
	}
	if(antsCreated>=antsTotal-1){
		printf("All ants created!\n",myData->id);
		pthread_cond_broadcast(&condition_var);
	}
	pthread_mutex_unlock(&creation_mutex);
	/*printf("All ants created!\n");
	pthread_mutex_lock(&creation_mutex);	
	pthread_cond_signal(&condition_var);
	pthread_mutex_unlock(&creation_mutex);*/

	for(i = 0; i < numAnts; i++){
		// printf("Global was : %d at point (%d,%d)\n", bestG->value,bestG->coos->x,bestG->coos->y);
		findGlobalBest(swarm[i], mode);
		// printf("NEW GLOBAL IS : %d at point (%d,%d)\n", bestG->value,bestG->coos->x,bestG->coos->y);
		//printf("%f\n", bestG->value);
	}

	// while(1){
	// 	sleep(1);
	// }

	for(i = 0; i < MAXITERATIONS; i++){		
		for(j = 0; j < numAnts; j++){			
			vel = calVelocity(swarm[j]);
			//printf("vel->x=%d vel->y=%d\n", vel->x, vel->y);	
			moveAnt(vel, &(swarm[j]), function, mode);			
		}		

		for(j = 0; j < numAnts; j++){
			// printf("Ant %d x=%d y=%d\n", j, swarm[j]->position->x, swarm[j]->position->y);
		}		

		//THREAD HAS TO WAIT FOR OTHER THREADS MOVING THEIR ANTS
		//WE NEED TO SELECT GLOBALBEST
		for(j = 0; j < numAnts; j++){
			findGlobalBest(swarm[j], mode);
		}		
	}	
}

void createThreads(int numThreads,int antsTotal,int tmpNumOfAnts, int function,int mode){

	threads = malloc(sizeof(pthread_t)*numThreads);

	int i,rc;

	TData* tData;
	
	for(i = 0; i < numThreads; i++){
		tData = (TData*)malloc(sizeof(TData));
		tData->id = i;
		tData->numAnts = antsTotal > tmpNumOfAnts ? ANTS_PER_THREAD : tmpNumOfAnts;
		//printf("%d\n", tData->numAnts);
		tData->function = function - 1;
		tData->mode = mode;
		//printf("%d\n", mode);
		antsTotal -= ANTS_PER_THREAD;
		// printf("Creating thread %d\n", i);
		rc = pthread_create(&threads[i], NULL, startPSO, (void*)tData);
		if(rc){
			printf("Error creating thread %d pthread_create got %d\n", i, rc);
			exit(-1);
		}
	}
}

int main(void){

	urandom = fopen("/dev/urandom","r");
	if(urandom == NULL){
		printf("Can't open /dev/urandom !!!\n");
		exit(-1);
	}	
	
	int i, rc, tmpNumOfAnts, function, mode;

	bestG = (BestGlobal*)malloc(sizeof(BestGlobal));
	bestG->value = 0;
	bestG->coos = (Point*)malloc(sizeof(Point));
	bestG->coos->x = 0;
	bestG->coos->y = 0;
	
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

	close(urandom);

	return 0;
}