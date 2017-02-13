#ifndef AUTOZOOM_H
#define AUTOZOOM_H
#include "statemachine.h"
#include "viewfinder.h"

/**
 * @class AutoZoom
 * Автоматически изменяет линейные размеры кадра в соответствии с размерами детектированного лица
 */
class AutoZoom : public MotionAutomata
{
    const Size maxRoiSize;   ///< Максимальный размер области захвата кадра. Равен размеру сжатого кадра или меньше.
    const float zoomThr;    ///< Триггер начала зуммирования
    const double face2shot; ///< Требуемое отношение высоты лица к высоте кадра
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
          face2shot(face2shot_)
    {
        speedInc=zoomSpeedInc_;
    }
    float update(const Rect &aim, ViewFinder &roi);
    float getStopThr() const;
};

#endif // AUTOZOOM_H
