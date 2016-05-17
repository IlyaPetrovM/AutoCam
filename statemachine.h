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
             } DYNAMIC_STATES;
class StateMachine
{
protected:
    DYNAMIC_STATES state; ///< Состояния движения
    double speedMin; ///< Минимальная скорость перемещения
    double speedMax; ///< Максимальная скорость перемещения
    double speedAim; ///< Желаемая скорость перемещения
    double speedInc; ///< Инкремент. Насколько скорость будет увеличена или уменьшена
    double accelTime; ///< Время ускорения
    double speed; ///< Текущая скорость
    float sign; ///< Знак изменения скорости (+/-)
public:
    StateMachine(double spdMin, double spdMax);
    /**
     * @brief getState
     * @return Текущее состояние движения
     */
    DYNAMIC_STATES getState(){
         return state;
    }
    /**
     * @brief getSign
     * @return Текущий знак движения
     */
    float getSign(){ return sign;}
    /**
     * @brief getSpeed
     * @return Текущую скорость
     */
    double getSpeed(){
        return speed;
    }
};

#endif // STATEMACHINE_H
