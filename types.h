#ifndef	__TYPES_H__
#define __TYPES_H__

typedef struct point{
	int x;
	int y;
} Point;

typedef struct ant{
	
	Point * position;
	float fitness;
	Point * velocity;
	Point * bestPosition;
	float bestFitness;	

}Ant;


#endif