#include <cmath>
#include "func.h"

double var1 = 0;
double var2 = 0;
double var3 = 0;

void func1(int j) {
    var1 += j;
}

void func2(int j) {
    var2 += j;
}

void func3(int j) {
    var3 += j;
}

struct Func1 : public Func {
    void func(int j) override;
};

struct Func2 : public Func {
    void func(int j) override;
};

struct Func3 : public Func {
    void func(int j) override;
};

void Func1::func(int j) {
    var1 += j;
}

void Func2::func(int j) {
    var2 += j;
}

void Func3::func(int j) {
    var3 += j;
}

Func1 myf1;
Func2 myf2;
Func3 myf3;

Func* genFunc(int j) {
    switch (j % 3) {
        case 0: return &myf1; break;
        case 1: return &myf2; break;
        case 2: return &myf3; break;
    }
    return nullptr;
}

struct Func4 : public Func1 {
    virtual void func(int j);
};
struct Func5 : public Func2 {
    virtual void func(int j);
};
struct Func6 : public Func3 {
    virtual void func(int j);
};

void Func4::func(int j) {
    var1 += j;
}

void Func5::func(int j) {
    var2 += j;
}

void Func6::func(int j) {
    var3 += j;
}

Func4 myf4;
Func5 myf5;
Func6 myf6;

Func* genFunc2(int j) {
    switch (j % 6) {
        case 0: return &myf1; break;
        case 1: return &myf2; break;
        case 2: return &myf3; break;
        case 3: return &myf4; break;
        case 4: return &myf5; break;
        case 5: return &myf6; break;
    }
    return nullptr;
}
