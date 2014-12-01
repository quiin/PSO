#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "defines.h"
#include "types.h"


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
		result += 100 * pow((getCooDim(i+ 1, ant) - pow((float)coo, 2.0)), 2.0) + pow((float)(1-coo), 2.0);
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

int getCooDim(int dimension, Ant* ant){
	switch (dimension){
		case 0: return ant->position->x;
		case 1: return ant->position->y;
		// case 2; return ant->position->z;
		// case 3; return ant->position->k;
		default: return -1;
	}
}