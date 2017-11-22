#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include <string>
#include "scene.h"
#include "output.h"
#include <opencv2/imgproc.hpp>
using namespace cv;
using namespace std;
class Camera
{
    int id;
    static int camCnt;
    float vx,vy,vz;

    unsigned int x,y,width,height;
    float z;

    unsigned int xmax,ymax,widthmax,heightmax;
    Scene *scene;
    Frame frameIn;
    Frame frameOut;
    float fric;
    void friction(float &vel);
    vector<Output*> port;
    void cutFrame();
    void sendFrame();
    void calcFrameSize();
public:
    Camera(Scene *_scene, const int _targetWidth, const int _targetHeight);
    ~Camera();
    void update();
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void zoomIn();
    void zoomOut();

    int getId() const;
    float getFric() const;
    void setFric(float value);
    void addPort(Output *_port);
    vector<Output *> getPorts() const;
};


#endif // CAMERA_H
