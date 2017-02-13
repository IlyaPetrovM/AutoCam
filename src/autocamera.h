#ifndef AUTOCAMERA_H
#define AUTOCAMERA_H
#include "autozoom.h"
#include "automotion.h"
#include "viewfinder.h"
#include <iostream>
/**
  @class AutoCamera
 * Класс AutoCamera на основе полученной цели плавно изменяет координаты и линейные размеры кадра
 */
class AutoCamera
{
    const double onePerc; ///< Один процент от ширины сжатого кадра
    const bool bZoom; ///< Использовать ли масштабирование. \c true - если использовать.
    const bool bMove; ///< Установить в значение \c true чтобы перемещать камеру
    const Size maxRoiSize; ///< Максимальный размер области захвата кадра. Равен размеру сжатого кадра или меньше.

    AutoPan moveX; ///< Механизм перемещения камеры по горизонтали
    AutoPan moveY; ///< Механизм перемещения камеры по вертикали
    AutoZoom zoom; ///< Механизм масштабирования кадра
    ViewFinder roi;

    /**
     * @brief topMiddleDec Ищет точку посередине прямоугольника, отстоящую от верха на одну треть (в относительных координатах)
     * @param[in] r Прямоугольник, в котором ищется точка
     * @return Точка, у которой \c x и \c y - отступы от левого верхнего края прямоугольника
     */
    inline Point topMiddleDec(const Rect2f& r) {return Point(cvRound((double)r.width*0.5) , cvRound((double)r.height/3.0));}
    /**
     * @brief topLeftDec Ищет левую верхнюю точку по правилу третей (в относительных координатах)
     * @param[in] r Прямоугольник, в котором ищется точка
     * @return Точка, у которой \c x и \c y - отступы от левого верхнего края прямоугольника
     */
    inline Point topLeftDec(const Rect2f& r) {return Point(cvRound((double)r.width/3.0) , cvRound((double)r.height/3.0));}
    /**
     * @brief topRightDec Ищет правую верхнюю точку по правилу третей (в относительных координатах)
     * @param[in] r Прямоугольник, в котором ищется точка
     * @return Точка, у которой \c x и \c y - отступы от левого верхнего края прямоугольника
     */
    inline Point topRightDec(const Rect2f& r){return Point(cvRound((double)r.width*2.0/3.0) , cvRound((double)r.height/3.0));}
    /**
     * @brief getGoldenPoint Ищет координаты прямоугольника так, чтобы внитри него располагалось лицо по правилу третей
     * @param[in] roi Область интереса, кадр
     * @param[in] face лицо
     * @return Абсолютные координаты нового положения кадра
     */
    Point getGoldenPoint(const Rect2f &roi, const Rect& face);
public:
    /**
     * @brief Конструктор
     * @param scale_ отношение линейных размеров кадра исходного видео и сжатого кадра
     * @param maxRoiSize_
     * @param maxStepX
     * @param maxStepY
     * @param zoomSpeedMin
     * @param zoomSpeedMax
     * @param zoomThr
     * @param zoomStopThr_
     * @param zoomSpeedInc_
     * @param face2shot
     * @param bZoom_
     * @param bMove_
     */
    AutoCamera(Size maxRoiSize_,
               double maxStepX,
               double maxStepY,
               double zoomSpeedMin,
               double zoomSpeedMax,
               double zoomThr,
               double zoomSpeedInc_,
               double face2shot,
               bool bZoom_,
               bool bMove_);
    /**
     * @brief Анализирует цель aim и на основе этого анализа перемещает и масштабирует кадр.
     * @param[in] aim - цель, которая должна быть захвачена кадром
     */
    void update(const Rect aim);
    /**
     * @brief масштабирует размеры ROI под реальные размеры, чтобы из исходного видео высокого качества вырезать нужную область.
     * @return
     */
    Rect2f getRoi() const;
    void setRoi(const Rect2f &value);
    AutoPan getMoveX() const;
    void setMoveX(const AutoPan &value);
    AutoPan getMoveY() const;
    void setMoveY(const AutoPan &value);
    AutoZoom getZoom() const;
    void setZoom(const AutoZoom &value);
};

#endif // AUTOCAMERA_H
