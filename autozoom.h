#ifndef AUTOZOOM_H
#define AUTOZOOM_H
#include "statemachine.h"


class AutoZoom : public StateMachine
{
    double face2shot;
    Size aspect;
    Size maxRoiSize;
    float stopThr; ///< Триггерное значение окончания зуммирования
    float zoomThr; ///< Триггер начала зуммирования
public:
    AutoZoom(double spdMin, double spdMax, Size maxRoiSize_, float zoomThr_, float stopThr_,float zoomSpeedInc_, double face2shot_)
        : StateMachine(spdMin,spdMax),
          maxRoiSize(maxRoiSize_),
          zoomThr(zoomThr_),
          stopThr(stopThr_),
          face2shot(face2shot_),
          aspect(getAspect(maxRoiSize_))
    {speedInc=((spdMax-spdMin)/zoomSpeedInc_);}
    /**
     * @brief getAspect Определяет соотношение сторон кадра в удобочитаемом виде
     * @param [in]sz размеры кадра
     * @return
     */
    inline static Size getAspect(const Size& sz){
        int g=gcd(sz.width,sz.height);
        return Size(sz.width/g,sz.height/g);
    }
    /**
     * @brief gcd Определяет наибольший общий делитель
     * @param [in]a
     * @param [in]b
     * @return Наибольший общий делитель a и b
     */
    static int gcd(int a,int b){
        int c;
        while (a != 0){
            c = a;
            a = b%a;
            b = c;
        }
        return b;
    }
    /**
     * @brief scaleRect
     * @param r
     * @param asp
     * @param sc
     */
    inline void scaleRect(Rect2f &r,const Size& asp, const float &sc=1.0){ /// from center
        r.height+=2*asp.height*sc;
        r.width+=2*asp.width*sc;
        r.x -= asp.width*sc;
        r.y -= asp.height*sc;
    }
    float update(const Rect &aim, Rect2f &roi);
    float getStopThr() const;
    void setStopThr(float value);
};

#endif // AUTOZOOM_H
