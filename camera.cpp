#include "camera.h"

int Camera::camCnt = 0;
int Camera::getId() const
{
    return id;
}

void Camera::update()
{
    x+=vx;
    y+=vy;
    z+=vz;
    friction(vx);
    friction(vz);
    friction(vz);

    calcFrameSize();

    scene->getFrame(frameIn);
    cutFrame();
    sendFrame();
}


float Camera::getFric() const
{
    return fric;
}

void Camera::setFric(float value)
{
    fric = value;
}

void Camera::addPort(Output *_port)
{
    port.push_back(_port);
}

vector<Output*> Camera::getPorts() const
{
    return port;
}

void Camera::friction(float &vel)
{
    if(vel>0)vel-=fric; else if(vel<0)vel+=fric;
}

void Camera::cutFrame()
{
    /// \todo frameOut = frameIn(Rect(x,y,width,height));
    frameOut = frameIn.clone();
}

Camera::Camera(Scene *_scene)
    : id(camCnt),
      scene(_scene),
      fric(0.5)
{
    camCnt++;
}

Camera::~Camera()
{
    for(size_t i=0;i<port.size();i++){
        delete port[i];
    }
    clog << "\tcamera #"<<id<< " deleted" <<endl;
}

void Camera::moveUp()
{
    vy++;
}

void Camera::moveDown()
{
    vy--;
}

void Camera::moveLeft()
{
    vx--;
}

void Camera::moveRight()
{
    vx++;
}

void Camera::zoomIn()
{
    vz++;
}

void Camera::zoomOut()
{
    vz--;
}

void Camera::sendFrame()
{
//    clog<<"cam "<< id<<" sends frame" << endl;
    for(size_t i=0; i<port.size();i++){
        port[i]->sendFrame(frameOut);
    }
}

void Camera::calcFrameSize()
{
    ///\todo
    /// z and width and height
}

