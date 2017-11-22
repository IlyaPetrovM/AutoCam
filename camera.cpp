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

    scene->getFrame(&frameIn);
    if(frameIn.cameOnTime()){
        cutFrame();
        sendFrame();
    }else{
        frameIn.drop();
    }
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
    Log::print(DEBUG,string(__FUNCTION__)+"1");

    static Mat tmp;
//    tmp = frameIn.getPixels().clone();
    resize(frameIn.getPixels(),tmp,Size(width,height)); //todo target width and height
    frameOut.setPixels(tmp);
    frameOut.setNum(frameIn.getNum());
    frameOut.setDeadline_us(frameIn.getDeadline_us());

    Log::print(DEBUG,string(__FUNCTION__)+"delete frameInPtr");

}

Camera::Camera(Scene *_scene, const int _targetWidth, const int _targetHeight)
    : id(camCnt),
      scene(_scene),
      fric(0.5),
      width(_targetWidth),
      height(_targetHeight)
{
    camCnt++;
}

Camera::~Camera()
{
    for(size_t i=0;i<port.size();i++){
        Log::print(DEBUG,"\tcamera #"+to_string(id)+" deleting port "+to_string(i) );
        delete port[i];
    }
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
    Log::print(DEBUG,string(__FUNCTION__)+" camera");
    for(size_t i=0; i<port.size();i++){
        port[i]->sendFrame(&frameOut);
    }
}

void Camera::calcFrameSize()
{
    ///\todo
    /// z and width and height
}

