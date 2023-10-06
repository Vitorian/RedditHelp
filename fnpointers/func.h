#pragma once

struct Func {
    virtual void func(int j) = 0;
};

void func1(int);
void func2(int);
void func3(int);

Func* genFunc(int);
Func* genFunc2(int);
