#include "autozoom.h"

float AutoZoom::update(const Rect& aim,
                       ViewFinder& roi)
{    
    float aimH = aim.height*face2shot;

    switch (state) {
    case STOP:
        if(aimH>roi.getHeight()*(1.0+zoomThr)){ /// zoomThr показывает насколько точно масштабирование/зум должны совпасть с целевым
            sign=1;
            state=BEGIN;
        }
        if(aimH<roi.getHeight()*(1.0-zoomThr)){
            sign=-1;
            state=BEGIN;
        }
        break;
    case BEGIN:
        speed+=speedInc;
        roi.scale(sign*speed);
        if(speed> speedMax) {state=MOVE;speed=speedMax;}
        if(roi.getHeight() >= maxRoiSize.height) {
            state=STOP;speed=speedMin;
        }
        break;
    case MOVE:
        roi.scale(sign*speed);
        if(aimH<roi.getHeight()*(1.0+zoomThr) && aimH>roi.getHeight()*(1.0-zoomThr))state=END;
        if(roi.getHeight() > maxRoiSize.height) {
            state=STOP;speed=speedMin;
        }
        break;
    case END:
        speed-=speedInc;
        roi.scale(sign*speed);
        if(speed< speedMin) {state=STOP; speed=speedMin;}
        if(roi.getHeight() > maxRoiSize.height || roi.getHeight() > maxRoiSize.width) {
            state=STOP;
            speed=speedMin;
        }
        break;
    }
    return speed;
}
