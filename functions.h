#ifndef	__DEFINES_H__
#define __DEFINES_H__

float sphere(Ant* ant);
float rosenbrock(Ant* ant);
float schwefel(Ant* ant);
float griewank(Ant* ant);
float rastrigin(Ant* ant);
float calFun(int fun,Ant* ant);
void minOrMax(Ant** antP, int fun, int mode);
int getCooDim(int dimension, Ant* ant)

#endif