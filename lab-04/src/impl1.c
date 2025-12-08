#include <math.h>
#include "calc.h"

float Derivative(float A, float deltaX) {
    if (deltaX == 0.0f) return NAN;
    float fA = cosf(A);
    float fAp = cosf(A + deltaX);
    return (fAp - fA) / deltaX;
}

int GCF(int A, int B) {
    if (A == 0) return (B >= 0) ? B : -B;
    if (B == 0) return (A >= 0) ? A : -A;
    if (A < 0) A = -A;
    if (B < 0) B = -B;
    while (B != 0) {
        int t = A % B;
        A = B;
        B = t;
    }
    return A;
}