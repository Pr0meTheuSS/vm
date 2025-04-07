#include <stdio.h>
#include <stdlib.h>

float a;
float b;
float c;
float P;
float S;
float alpha;
float beta;
float gamma;

// Функция для вычисления calculatePerimeter
float calculatePerimeter(float a, float b, float c) {
return a + b + c;
}

// Функция для вычисления calculateArea
float calculateArea(float a, float b, float c) {
float s = P / 2.0;
 return sqrt(s * (s - a) * (s - b) * (s - c));
}

// Функция для вычисления calculateAlpha
float calculateAlpha(float a, float b, float c) {
return acos((b*b + c*c - a*a) / (2.0 * b * c));
}

// Функция для вычисления calculateBeta
float calculateBeta(float a, float b, float c) {
return acos((a*a + c*c - b*b) / (2.0 * a * c));
}

// Функция для вычисления calculateGamma
float calculateGamma(float a, float b, float c) {
return acos((a*a + b*b - c*c) / (2.0 * a * b));
}
int main() {
    a = 3.0;
    b = 4.0;
    c = 5.0;
    P = 12.0;
	S = calculateArea(a, b, c);
	alpha = calculateAlpha(a, b, c);
	beta = calculateBeta(a, b, c);
	gamma = calculateGamma(a, b, c);

    return 0;
}
