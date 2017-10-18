#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include <string>
#include "scene.h"
using namespace std;
class Camera
{
    int id;
    static int camCnt;
    float vx,vy,vz;

    int x,y,width,height;
    float z;
    string port; //where the cam send frames
    Scene *scene;
    float fric;
    void friction(float &vel);
public:
    Camera(Scene *_scene, string _port);
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
    string getPort() const;
    void setPort(const string &value);
};


#endif // CAMERA_H
