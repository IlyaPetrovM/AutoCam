#ifndef AUTOMOTION_H
#define AUTOMOTION_H
#include "statemachine.h"
#include "math.h"
/**
 * @brief The autoMotion class
 *
 * Плавное перемещение виртуальной камеры с использованием нескольких состояний
 */
class AutoMotion : public StateMachine
{

public:
     /**
     * @brief Конструктор
     * @param [in]spdMin минимальная скорость
     * @param [in]spdMax максимальная скорость
     */
    AutoMotion(double spdMin,double spdMax) : StateMachine(spdMin, spdMax){}
    /**
     * @brief update
     * Обновить координаты в соответствии с текущей скоростьюи состоянием
     * @param [in,out]x
     * @param [in]aim
     * @param [in]precision
     * @param outOfRoi
     * @return Текущую скорость изменения координаты
     */
    float update(float& x,const int& aim, const double& precision, const bool outOfRoi);
};

#endif // AUTOMOTION_H
