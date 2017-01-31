#include "autozoom.h"

float AutoZoom::update(const Rect& aim,
                       Rect2f& roi)
{    
    float aimH = aim.height*face2shot;

    switch (state) {
    case STOP:
        if(aimH>roi.height*(1.0+zoomThr)){ /// zoomThr показывает насколько точно масштабирование/зум должны совпасть с целевым
            sign=1;
            state=BEGIN;
        }
        if(aimH<roi.height*(1.0-zoomThr)){
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
        if(aimH<roi.height*(1.0+zoomThr) && aimH>roi.height*(1.0-zoomThr))state=END;
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
        if(roi.height > maxRoiSize.height || roi.width > maxRoiSize.width) {
            roi.height=maxRoiSize.height;
            roi.width=maxRoiSize.width;
            state=STOP;
            speed=speedMin;
        }
        break;
    }
    return speed;
}
