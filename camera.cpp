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

string Camera::getPort() const
{
    return port;
}

void Camera::setPort(const string &value)
{
    port = value;
}
void Camera::friction(float &vel)
{
    if(vel>0)vel-=fric; else if(vel<0)vel+=fric;
}

Camera::Camera(Scene *_scene, string _port)
    : id(camCnt),
      scene(_scene),
      port(_port),
      fric(0.5)
{
    camCnt++;
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
    //crop the scene using coordinates
    clog << " cam"<<id<<" send frame with coords: ("<<x<<","<<y<<","<<z<<") to "<< port <<endl;
}

