#ifndef CALC_H
#define CALC_H

#ifdef __cplusplus
extern "C" {
#endif

/* производная cos(x) */
float Derivative(float A, float deltaX);

/* наибольший общий делитель */
int GCF(int A, int B);

#ifdef __cplusplus
}
#endif

#endif /* CALC_H */
