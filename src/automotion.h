#ifndef AUTOMOTION_H
#define AUTOMOTION_H
#include "statemachine.h"
#include "math.h"
/**
 * \class AutoPan
 * Измененяет одну из координат вырезаемого кадра
 */
class AutoPan : public MotionAutomata
{
    const float maxVal;
public:
     /**
     * @brief Конструктор
     * @param[in] spdMin минимальная скорость
     * @param[in] spdMax максимальная скорость
     */
    AutoPan(double spdMin,double spdMax, float maxVal_) : MotionAutomata(spdMin, spdMax), maxVal(maxVal_){}
    /**
     * Обновить координаты в соответствии с текущей скоростьюи состоянием
     * @param[in,out] x
     * @param[in] aim
     * @param[in] precision
     * @param outOfRoi
     * @return Текущую скорость изменения координаты
     */
    float update(const float &preVal_, const int& aim, const double& precision, const bool outOfRoi);
    void stop(){
        state=STOP;
        speed=speedMin;
    }
};

#endif // AUTOMOTION_H
