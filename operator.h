#ifndef OPERATOR_H
#define OPERATOR_H
#include <iostream>
#include "camera.h"
#include "face.h"
#include "scene.h"
#include <atomic>
typedef enum {AUTO,MANUAL} workType;
class Operator
{
    static int opCnt;
    int id;
    Face face;
    Camera camera;
    Scene *scene;
    bool idealComposition;
    workType typeOfWork;
    void workManual();
    void workAuto();
    atomic<char> cmd;
public:
    Operator(Scene *s, Camera *cam, Face *target);
    ~Operator();
    void work();
    Face getFace() const;
    void setFace(const Face &value);
    int getId() const;
    workType getTypeOfWork() const;
    void setTypeOfWork(const workType &value);
    void sendCommand(char _cmd);
};

#endif // OPERATOR_H
