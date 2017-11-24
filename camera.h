#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include <string>
#include "scene.h"
#include "output.h"

using namespace cv;
using namespace std;
class Camera
{
    int id;
    static int camCnt;
    double vx,vy,vz;


    unsigned int x,y,width,height;
    double z;
    double maxZoom;
    double minZoom;
    unsigned int maxX,maxY,maxWidth,maxHeight;
    Scene *scene;
    Frame frameIn;
    Frame frameOut;
    double safeMargin;
    double fric;
    void friction(double &vel);
    vector<Output*> port;
    void cutFrame();
    void sendFrame();
    void calcFrameSize();
public:
    Camera(Scene *_scene, double _safeMargin=0.1, double _friction=0.1);
    ~Camera();
    void update();
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void zoomIn();
    void zoomOut();

    int getId() const;
    double getFric() const;
    void setFric(double _friction);
    void addPort(Output *_port);
    vector<Output *> getPorts() const;
    double getSafeMargin() const;
    void setSafeMargin(double value);
};


#endif // CAMERA_H
