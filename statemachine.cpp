#include "statemachine.h"

MotionAutomata::MotionAutomata(double spdMin,double spdMax)
{
    state = STOP;
    speedMin=spdMin;
    speedMax=spdMax;
    speed=spdMin;
}
