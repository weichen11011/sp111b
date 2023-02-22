#include <stdio.h>

double integrate(double (*f)(double),double a, double b) {
    double n = 1000;
    double dx = (b-a)/n;
    double sum=0;
    for (double i=a;i<b;i+=dx){
        sum = sum + f(i)*dx;
    }
    return sum;
}

double square(double x) {
    return x*x;
}

int main() {
    printf("integrate(square, 0.0, 2.0)=%f\n", integrate(square, 0.0, 2.0));
}