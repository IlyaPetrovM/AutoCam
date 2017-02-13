#ifndef AIM_H
#define AIM_H

#include <opencv2/core.hpp>
using namespace cv;
class ViewFinder : Rect2f
{
    const Rect scene;
    const Size aspect;      ///< Cоотношение сторон кадра в удобочитаемом виде
    float x,y,width,height;
    /**
     * @brief gcd Определяет наибольший общий делитель
     * @param[in] a, b числа
     * @return Наибольший общий делитель \c a и \c b
     */
    int gcd(int a,int b){
        int c;
        while (a != 0){
            c = a;
            a = b%a;
            b = c;
        }
        return b;
    }
    /**
     * @brief getAspect Определяет соотношение сторон кадра в удобочитаемом виде
     * @param[in] sz размеры кадра
     * @return
     */
    inline Size getAspect(const Size& sz){
        int g=gcd(sz.width,sz.height);
        return Size(sz.width/g,sz.height/g);
    }
    bool setWidth(float _width);
    bool setHeight(float _height);
public:
    ViewFinder(Rect scene_);
    /**
     * Увеличить лиенейные размеры прямоугольника \c r в \c sc раз
     * @param r
     * @param asp (aspect) соотношение сторон прямоугольника в удобочитаемом виде
     * @param sc Отношение линейных размеров большего прямоугольника меньшему
     */
    void scale(const float &sc=1.0);

    float getWidth() const;
    float getY() const;
    float getX() const;
    bool setY(float _y);
    bool setX(float _x);
    Rect getRect() const {return Rect((int)x,(int)y,(int)width,(int)height);}
    Rect getRect2f() const {return Rect2f(x,y,width,height);}
    float getHeight() const;
};

#endif // AIM_H
