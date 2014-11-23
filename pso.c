#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "stack.c"

#define MAXLNEQ 200

char* eq[MAXLNEQ];

typedef struct point{
	int x;
	int y;
} Point;

typedef struct thread_data{
	int id;
	int numAnts;
} TData;

float parseFunction(char* eq);

void* startPSO(void* tData){

}

int main(void){
	
	int i, rc, numberOfAnts, rangeX, rangeY, numThreads, tmpNumOfAnts;
	
	Point* start = (Point*)malloc(sizeof(Point));
	
	printf("Cuantas hormigas quieres tener?\n");
	scanf("%d", &numberOfAnts);
	printf("Dame la coordenada en x de tu punto incial de tu mapa: ");
	scanf("%d", &(start->x));
	printf("Dame la coordenada en y de tu punto inicial de tu mapa: ");
	scanf("%d", &(start->y));
	printf("Cual es tu rango en x? ");
	scanf("%d" , &rangeX);
	printf("Cual es tu rango en y? ");
	scanf("%d", &rangeY);
	printf("Que ecuacion quieres utilizar?");
	scanf("%s", eq);
	printf("%s\n", &eq);
	printf("El resultado de la operación es: %f\n",parseFunction(*eq));
	
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

float parseFunction(char* eq){

}