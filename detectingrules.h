#ifndef DETECTINGRULES_H
#define DETECTINGRULES_H
#include "face.h"
#include "math.h"
#include <iostream>
class DetectingRules
{
    int minDist;
public:
    DetectingRules(int _minDist);
    bool near(Face f1, Face f2);
};

#endif // DETECTINGRULES_H
