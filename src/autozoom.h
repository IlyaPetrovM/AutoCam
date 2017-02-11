#ifndef AUTOZOOM_H
#define AUTOZOOM_H
#include "statemachine.h"

/**
 * @class AutoZoom
 * Автоматически изменяет линейные размеры кадра в соответствии с размерами детектированного лица
 */
class AutoZoom : public MotionAutomata
{
    const Size maxRoiSize;   ///< Максимальный размер области захвата кадра. Равен размеру сжатого кадра или меньше.
    const float zoomThr;    ///< Триггер начала зуммирования
    const double face2shot; ///< Требуемое отношение высоты лица к высоте кадра
    const Size aspect;      ///< Cоотношение сторон кадра в удобочитаемом виде
    /**
     * @brief gcd Определяет наибольший общий делитель
     * @param[in] a, b числа
     * @return Наибольший общий делитель \c a и \c b
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
     * Увеличить лиенейные размеры прямоугольника \c r в \c sc раз
     * @param r
     * @param asp (aspect) соотношение сторон прямоугольника в удобочитаемом виде
     * @param sc Отношение линейных размеров большего прямоугольника меньшему
     */
    inline void scaleRect(Rect2f &r, const float &sc=1.0){ /// from center
        r.height+=2*aspect.height*sc;
        r.width+=2*aspect.width*sc;
        r.x -= aspect.width*sc;
        r.y -= aspect.height*sc;
    }
public:
    /**
     * @brief Конструктор
     * @param spdMin Минимальная скорость масштабирования
     * @param spdMax Максимальная скорость масштабирования
     * @param maxRoiSize_ Максимальные линейные размеры захватываемой области
     * @param zoomThr_ Максимальная разница между высотами лица человека и захватываемого кадра. При превышении этого порога начинается масштабирование
     * @param stopThr_ Триггерное значение окончания зуммирования
     * @param zoomSpeedInc_ Инкремент. Насколько скорость будет увеличена или уменьшена за одну итерацию
     * @param face2shot_ Отношение высоты кадра к высоте лица. Данный класс должен соблюдать это отношение и изменять размер кадра при несоответствии этому отношению.
     */
    AutoZoom(double spdMin, double spdMax, Size maxRoiSize_, float zoomThr_,float zoomSpeedInc_, double face2shot_)
        : MotionAutomata(spdMin,spdMax),
          maxRoiSize(maxRoiSize_),
          zoomThr(zoomThr_),
          face2shot(face2shot_),
          aspect(getAspect(maxRoiSize_))
    {
        speedInc=zoomSpeedInc_;
    }
    /**
     * @brief getAspect Определяет соотношение сторон кадра в удобочитаемом виде
     * @param[in] sz размеры кадра
     * @return
     */
    inline static Size getAspect(const Size& sz){
        int g=gcd(sz.width,sz.height);
        return Size(sz.width/g,sz.height/g);
    }

    float update(const Rect &aim, Rect2f &roi);
    float getStopThr() const;
};

#endif // AUTOZOOM_H
