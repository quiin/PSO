#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <float.h>
#include "functions.c"
#include "Ant.c"

typedef struct thread_data{
	int id;
	int numAnts;
} TData;


void initSwarm(Ant swarm[],int swarmSize);


void* startPSO(void* tData){

}

int main(int argc, char *argv[]){

	if(argc<=1){
		printf("Ejecución erronea\n");		
		printf("Uso: ./pso 1\n");
		printf("***Funciones***\n");
		puts("1.- Sphere");
		puts("2.- Rosenbrock");
		puts("3.- Schwefel");
		puts("4.- Griewank");
		puts("5.- Rastrigin");
		exit(-1);
	}
	int i=0;
	
	int choice = atoi(argv[1]);	

	int rc, numberOfAnts, dimension, numThreads, tmpNumOfAnts;
	
	// Point* start = (Point*)malloc(sizeof(Point));
	
	printf("Cuantas hormigas quieres tener?\n");
	scanf("%d", &numberOfAnts);
	printf("Dimensión del espacio?");
	scanf("%d",&dimension);
	/*printf("Dame la coordenada en x de tu punto incial de tu mapa: ");
	scanf("%d", &(start->x));
	printf("Dame la coordenada en y de tu punto inicial de tu mapa: ");
	scanf("%d", &(start->y));*/
	
	
	Ant swarm [numberOfAnts];
	int ** space = (int**)malloc(sizeof(int));
	initSwarm(swarm,numberOfAnts);
	
	/*double [dimension] bestGlobalPosition;
	double [DBL_MAX] bestGlobalValue;*/

	
	tmpNumOfAnts = numberOfAnts / 10;
	numThreads = tmpNumOfAnts + 1;
	pthread_t threads[numThreads];
	TData* tData;
	
	for(i = 0; i < numThreads; i++){
		tData = (TData*)malloc(sizeof(TData));
		tData->id = i;
		tData->numAnts = numberOfAnts > tmpNumOfAnts ? tmpNumOfAnts : numberOfAnts;
		numberOfAnts -= tmpNumOfAnts;
		printf("Creating thread %d\n", i);
		rc = pthread_create(&threads[i], NULL, startPSO, (void*)tData);
		if(rc){
			printf("Error creating thread %d pthread_create got %d\n", i, rc);
			exit(-1);
		}
	}

	for(i = 0; i < numThreads; i++){
		rc = pthread_join(threads[i], NULL);
		if(rc){
			printf("Error joining thread %d pthread_join got %d\n", i, rc);
			exit(-1);
		}
	}
	return 0;
}


void initSwarm(Ant swarm[],int swarmSize){
//initAnt(Point * position,double fitness,Point* velocity, Point* bestPosition, double bestFitness){
	int i,j,k;
	for(i=0;i<swarmSize-1;i++){
		Point* position = (Point*)malloc(sizeof(Point));
		position -> x = 0;
		position -> y = 0;
		Ant* ant = initAnt(position,0,position,position,0);
		printf("Position x %d y %d\n",ant->position->x,ant->position->y);
	}


}