#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "types.h"



Ant* initAnt(Point * position,double fitness,Point* velocity, Point* bestPosition, double bestFitness){
	Ant* ant = (Ant*) malloc(sizeof(Ant));
	ant->position = position;
	ant->fitness=fitness;
	ant->velocity=velocity;
	ant->bestPosition = bestPosition;
	ant->bestFitness = bestFitness;
	return ant;
}



