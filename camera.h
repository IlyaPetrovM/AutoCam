#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include <string>
#include "scene.h"
#include "output.h"
using namespace std;
class Camera
{
    int id;
    static int camCnt;
    float vx,vy,vz;

    int x,y,width,height;
    float z;
    Scene *scene;
    Mat frame;
    float fric;
    void friction(float &vel);
    vector<Output*> port;
public:
    Camera(Scene *_scene);
    ~Camera();
    void update();
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void zoomIn();
    void zoomOut();
    void sendFrame();

    int getId() const;
    float getFric() const;
    void setFric(float value);
    void addPort(Output *_port);
    vector<Output> getPorts() const;
};


#endif // CAMERA_H
