#include "automotion.h"

float AutoPan::update(float &x, const int &aim, const double &precision, const bool outOfRoi){
        switch (state) {/// \todo test this code!!
    case STOP:
        if(outOfRoi && abs(aim-cvRound(x))>2*precision){
            if(aim>cvRound(x)) sign=1.0; else sign=-1.0;
            if(abs(aim-cvRound(x))<1.5*precision){
                speedAim = speedMax/3.0;
            }else{
                speedAim = speedMax;
            }
            accelTime = precision*2.0/speedAim;
            speedInc=(speedAim-speedMin)/accelTime;
            state = BEGIN;
        }
        break;
    case BEGIN:
        speed+=speedInc;
        x += sign*speed;
        if(speed>speedAim) {state=MOVE;speed=speedAim;}
        break;
    case MOVE:
        x += sign*speed;
        if(abs(aim-cvRound(x))<precision) state=END;
        break;
    case END:
        speed-=speedInc;
        x += sign*speed;
        if(speed<speedMin) {state=STOP; speed=speedMin;}
        break;
    }
    return speed;
}
