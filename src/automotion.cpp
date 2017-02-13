#include "automotion.h"

float AutoPan::update(const float &preVal_, const int &aim, const double &precision, const bool outOfRoi){
    float val=preVal_;
        switch (state) {/// \todo test this code!!
    case STOP:
        if(outOfRoi && abs(aim-cvRound(val))>2*precision){
            if(aim>cvRound(val)) sign=1.0; else sign=-1.0;
            if(abs(aim-cvRound(val))<1.5*precision){
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
        val += sign*speed;
        if(speed>speedAim) {state=MOVE;speed=speedAim;}
        if(val<=0 || val >= maxVal) {stop();}
        break;
    case MOVE:
        val += sign*speed;
        if(abs(aim-cvRound(val))<precision) state=END;
        if(val<=0 || val >= maxVal) {stop();}
        break;
    case END:
        speed-=speedInc;
        val += sign*speed;
        if(speed<speedMin) {state=STOP; speed=speedMin;}
        if(val<=0 || val >= maxVal) {stop();}
        break;
    }
    return val;
}
