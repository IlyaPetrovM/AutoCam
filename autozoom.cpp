#include "autozoom.h"


float AutoZoom::getStopThr() const
{
    return stopThr;
}
float AutoZoom::update(const Rect& aim,
                       Rect2f& roi)
{    
    float aimH = aim.height*face2shot;

    switch (state) {
    case STOP:
        if(aimH>roi.height*zoomThr){
            sign=1;
            state=BEGIN;
        }
        if(aimH<roi.height/zoomThr){
            sign=-1;
            state=BEGIN;
        }
        break;
    case BEGIN:
        speed+=speedInc;
        scaleRect(roi,sign*speed);
        if(speed> speedMax) {state=MOVE;speed=speedMax;}
        if(roi.height > maxRoiSize.height) {
            roi.height=maxRoiSize.height;
            roi.width=maxRoiSize.width;
            state=END;
        }
        break;
    case MOVE:
        scaleRect(roi,sign*speed);
        if((abs(aimH-roi.height) < stopThr) || cvRound(roi.height) <= aim.height)state=END;
        if(roi.height > maxRoiSize.height) {
            roi.height=maxRoiSize.height;
            roi.width=maxRoiSize.width;
            state=END;
        }
        break;
    case END:
        speed-=speedInc;
        scaleRect(roi,sign*speed);
        if(speed< speedMin) {state=STOP; speed=speedMin;}
        if(roi.height > maxRoiSize.height) {
            roi.height=maxRoiSize.height;
            roi.width=maxRoiSize.width;
            state=STOP;
            speed=speedMin;
        }
        break;
    }
    return speed;
}
