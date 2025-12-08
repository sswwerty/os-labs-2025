#include <math.h>
#include "calc.h"

float Derivative(float A, float deltaX) {
    if (deltaX == 0.0f) return NAN;
    float fAp = cosf(A + deltaX);
    float fAm = cosf(A - deltaX);
    return (fAp - fAm) / (2.0f * deltaX);
}

int GCF(int A, int B) {
    if (A == 0) return (B >= 0) ? B : -B;
    if (B == 0) return (A >= 0) ? A : -A;
    if (A < 0) A = -A;
    if (B < 0) B = -B;
    int m = (A < B) ? A : B;
    for (int d = m; d >= 1; --d) {
        if (A % d == 0 && B % d == 0) return d;
    }
    return 1;
}