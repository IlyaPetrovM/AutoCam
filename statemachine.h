#ifndef STATEMACHINE_H
#define STATEMACHINE_H
#include "stdlib.h"
#include "opencv2/core.hpp"
using namespace cv;
/// Состояния движения
typedef enum {STOP, ///< Движение прекращено
              BEGIN, ///< Разгон
              MOVE, ///< Движение с постоянной максимальной скоростью
              END ///< Торможение
             } MOTION_STATES;
class MotionAutomata
{
protected:
    MOTION_STATES state; ///< Состояния движения
    double speedMin; ///< Минимальная скорость перемещения
    double speedMax; ///< Максимальная скорость перемещения
    double speedAim; ///< Желаемая скорость перемещения
    double speedInc; ///< Инкремент. Насколько скорость будет увеличена или уменьшена
    double accelTime; ///< Время ускорения
    double speed; ///< Текущая скорость
    float sign; ///< Знак изменения скорости (+/-)
public:
    /**
     * @brief Конструктор
     * @param spdMin минимальная скорость движения
     * @param spdMax максимальная скорость движения
     */
    MotionAutomata(double spdMin, double spdMax);
    /**
     * @brief getState
     * @return Текущее состояние движения
     */
    MOTION_STATES getState(){
         return state;
    }
    MotionAutomata operator =(MotionAutomata m){
        return m;
    }

    /**

     * @return Знак изменения скорости в данный момент
     */
    float getSign(){ return sign;}
    /**
     * @return Текущую скорость
     */
    double getSpeed(){
        return speed;
    }
};

#endif // STATEMACHINE_H
