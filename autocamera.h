#ifndef AUTOCAMERA_H
#define AUTOCAMERA_H
#include "autozoom.h"
#include "automotion.h"
#include <iostream>

class AutoCamera
{

    AutoMotion moveX;
    AutoMotion moveY;
    AutoZoom zoom;

    /*const*/ double onePerc;
    /*const*/ bool bZoom;
    /*const*/ bool bMove;
    Rect2f roi;
    /*const*/ Size maxRoiSize;

    Point gp; /// \todo 18.05.2016 class AutoCamera

    /**
     * @brief topMiddleDec Ищет точку посередине прямоугольника, отстоящую от верха на одну треть (в относительных координатах)
     * @param [in]r Прямоугольник, в котором ищется точка
     * @return Точка, у которой x и y - отступы от левого верхнего края прямоугольника
     */
    inline Point topMiddleDec(const Rect2f& r) {return Point(cvRound((double)r.width*0.5) , cvRound((double)r.height/3.0));}
    /**
     * @brief topLeftDec Ищет левую верхнюю точку по правилу третей (в относительных координатах)
     * @param [in]r Прямоугольник, в котором ищется точка
     * @return Точка, у которой x и y - отступы от левого верхнего края прямоугольника
     */
    inline Point topLeftDec(const Rect2f& r) {return Point(cvRound((double)r.width/3.0) , cvRound((double)r.height/3.0));}
    /**
     * @brief topRightDec Ищет правую верхнюю точку по правилу третей (в относительных координатах)
     * @param [in]r Прямоугольник, в котором ищется точка
     * @return Точка, у которой x и y - отступы от левого верхнего края прямоугольника
     */
    inline Point topRightDec(const Rect2f& r){return Point(cvRound((double)r.width*2.0/3.0) , cvRound((double)r.height/3.0));}
    /**
     * @brief getGoldenPoint Ищет координаты прямоугольника так, чтобы внитри него располагалось лицо по правилу третей
     * @param [in]roi Область интереса, кадр
     * @param [in]face лицо
     * @return Абсолютные координаты нового положения кадра
     */
    Point getGoldenPoint(const Rect2f& roi,const Rect& face);
public:
    AutoCamera(Size maxRoiSize_,
               double maxStepX,
               double maxStepY,
               double zoomSpeedMin,
               double zoomSpeedMax,
               double zoomThr,
               double zoomStopThr_,
               double zoomSpeedInc_,
               double face2shot,
               bool bZoom_,
               bool bMove_);
    void update(const Rect& aim);
    Rect2f getRoiFullSize(const double& scale) {
        return Rect2f(Point(roi.x*scale,roi.y*scale),Size(roi.width*scale,roi.height*scale));
    }
    Rect2f getRoi() const;
    void setRoi(const Rect2f &value);
    AutoMotion getMoveX() const;
    void setMoveX(const AutoMotion &value);
    AutoMotion getMoveY() const;
    void setMoveY(const AutoMotion &value);
    AutoZoom getZoom() const;
    void setZoom(const AutoZoom &value);
};

#endif // AUTOCAMERA_H
