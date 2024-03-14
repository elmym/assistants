#include <iostream>

class Approx {
public:
    Approx(double v);

    Approx& margin(double mar);
    Approx& epsilon(double eps);    

    friend bool operator==(double a, const Approx& appr);
private:
    double _val;
    double _mar;
};

Approx::Approx(double v) {
    _val = v;
    _mar = std::numeric_limits<double>::epsilon();
}

Approx& Approx::margin(double mar) {
    _mar = mar;
    return *this;
}

Approx& Approx::epsilon(double eps) {
    _mar = _val * eps;
    return *this;
}

bool operator == (double a, const Approx& appr) {
    if (a > appr._val - appr._mar && a < appr._val + appr._mar) return true;
    else return false;
}

int main()
{
    if (1.9 == Approx(1.9)) std::cout << 1;
    if (1.9009 == Approx(1.9)) std::cout << 2;
    if (1.9009 == Approx(1.9).margin(0.001)) std::cout << 3;
    if (1.9009 == Approx(1.9).margin(0.0007)) std::cout << 4;
    if (1.9009 == Approx(1.9).epsilon(0.0007)) std::cout << 5;
}
