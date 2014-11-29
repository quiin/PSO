#ifndef	__TYPES_H__
#define __TYPES_H__

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


#endif