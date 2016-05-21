#include "autocamera.h"

AutoCamera::AutoCamera(double scale_, Size maxRoiSize_, double maxStepX,
                       double maxStepY,
                       double zoomSpeedMin,
                       double zoomSpeedMax,
                       double zoomThr,
                       double zoomStopThr_,
                       double zoomSpeedInc_,
                       double face2shot,
                       bool bZoom_,
                       bool bMove_) :
    scale(scale_),
    maxRoiSize(maxRoiSize_),
    onePerc((double)maxRoiSize_.width/100.0),
    zoom(zoomSpeedMin,zoomSpeedMax,maxRoiSize_,zoomThr,cvRound(zoomStopThr_*((double)maxRoiSize_.width/100.0)),zoomSpeedInc_,face2shot),
    roi(Rect2f(Point(0,0),maxRoiSize_)),
    moveX(0.0,maxStepX*(double)maxRoiSize_.width/100.0),
    moveY(0.0,maxStepY*(double)maxRoiSize_.width/100.0),
    bZoom(bZoom_), bMove(bMove_)
{
}


void AutoCamera::update(const Rect &aim){
    static Point gp;
    register bool outOfRoi;
    if(bZoom){
        zoom.update(aim,roi);
    }

    if(bMove) {
        outOfRoi = ((Rect2f)aim&roi).area()<aim.area();
        gp = getGoldenPoint(roi,aim);
        moveX.update(roi.x,gp.x,roi.width/15.0,outOfRoi);
        moveY.update(roi.y,gp.y,roi.height/3.0,outOfRoi);
    }

    if(roi.x<0) roi.x = 0;
    if(maxRoiSize.width < roi.x+roi.width)
        roi.x = maxRoiSize.width-roi.width;
    if(roi.y<0)roi.y = 0;
    if(maxRoiSize.height < roi.y+roi.height)
        roi.y=maxRoiSize.height-roi.height;
}


Rect2f AutoCamera::getRoi() const
{
    return roi;
}

AutoPan AutoCamera::getMoveX() const
{
    return moveX;
}

AutoPan AutoCamera::getMoveY() const
{
    return moveY;
}

AutoZoom AutoCamera::getZoom() const
{
    return zoom;
}
Point AutoCamera::getGoldenPoint(const Rect2f &roi, const Rect &face){
    Point target;
    if(cvRound((float)roi.width/3.0) - face.width < 0 ) /// если лицо крупное, то держать его в центре кадра
        target = topMiddleDec(roi);
    else if(face.x+cvRound((float)face.width/2.0) < topMiddleDec(roi).x
            && face.x < roi.x+topLeftDec(roi).x)
        target = topLeftDec(roi);
    else if(face.x+face.width > roi.x+topRightDec(roi).x) // Камера посередине не будет реагировать
        target = topRightDec(roi);
    else
        target = topMiddleDec(roi);

    Point result = (face+topMiddleDec(face) - target).tl();// Должна быть зависимость только от размеров ROI
    return result;
}
