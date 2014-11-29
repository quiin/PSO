#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"


#define PI 3.141591

float sphere(int size,Ant* swarm []){

	float result;
	int i;
	result=0.0;
	for(i=0;i<size-1;i++){
		Ant* ant = swarm[i];
		result+= ant->fitness * ant->fitness;
	}
	return result;
}

float schwefel(int size,Ant* swarm[]){
	float result=0.0;
	int i;
	for(i=0;i<size;i++){
		Ant* ant = swarm[i];
		result+= ant->fitness*sin(sqrt(ant->fitness));
	}
	return -result;
}

float griewank(int size, Ant* swarm[]){
	int i;
	float result_s=0.0;
	float result_p=1.0;
	for(i=0;i<size;i++){
		Ant* ant = swarm[i];
		result_s+= ant->fitness*ant->fitness;
		result_p+= cos(ant->fitness/sqrt(i+1));
	}
	result_s = result_s/4000.0 +result_p+1;
	return result_s;

}

float rastrigrin (int size,Ant* swarm[]){
	int i;
	float result=0.0;
	for(i=0;i<size;i++){
		Ant* ant = swarm[i];
		float x = ant->fitness;
		result+=x*x - 10*cos(2*PI*x);
	}
	return 10+result;
}

float rosenbrock(int size, Ant* swarm[]){
	int i;
	float result=0.0;
	for(i=0;i<size;i++){
		Ant * ant = swarm[i];
		float x = ant->fitness;
		result+=100*(x+1-(x*x)) +(x+1)*(x+1);
		result *=x;
	}

	return result;
}