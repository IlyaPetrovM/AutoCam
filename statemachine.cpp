#include "statemachine.h"

StateMachine::StateMachine(double spdMin,double spdMax)
{
    state = STOP;
    speedMin=spdMin;
    speedMax=spdMax;
    speed=spdMin;
}
