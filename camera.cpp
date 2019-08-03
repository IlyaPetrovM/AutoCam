#include "camera.h"

Camera::Camera(Scene *_scene, double _safeMargin, double _friction)
    : id(camCnt),
      scene(_scene),
    realHeight(_scene->getHeight()),
    realWidth(_scene->getWidth())
{
    camCnt++;
    maxZoom = 4.0;
    minZoom = 1.0;
    setFric(_friction);
    setSafeMargin(_safeMargin);
    vz=0;
    vx=0;
    vy=0;
    z=minZoom;
    x=maxX;
    y=maxY;
    width = maxWidth;
    height = maxHeight;
}

void Camera::update()
{
    friction(vx);
    friction(vy);
    friction(vz);


    setX(getX() + vx);
    setY(getY() + vy);

//    if(getX()>= maxX and getY()>=maxY){
        setX(getX() + 16.0 * vz*0.1);
        setY(getY() + 9.0 * vz*0.1);
//    }
//    if(getX()+getWidth() <= maxWidth and getY()+getHeight() <= maxHeight){
        setWidth(getWidth()- 2.0 * 16.0 * vz*0.1);
        setHeight(getHeight() - 2.0 * 9.0 * vz*0.1);
//    }


    bool wasFrame=scene->getFrame(&frameIn);
    if(frameIn.cameOnTime() && wasFrame){
        cutFrame();
        sendFrame();
    }else{
        frameIn.drop(__FUNCTION__);
    }
}

int Camera::camCnt = 0;
int Camera::getId() const
{
    return id;
}

double Camera::getFric() const
{
    return fric;
}

void Camera::setFric(double _friction)
{
    if(_friction-0.1 < 0.00001)
        fric=0.1;
    else if(_friction >= 1.0)
        fric=0.99;
    else
        fric=_friction;
}

void Camera::addPort(Output *_port)
{
    port.push_back(_port);
}

vector<Output*> Camera::getPorts() const
{
    return port;
}

double Camera::getSafeMargin() const
{
    return safeMargin;
}

void Camera::setSafeMargin(double value)
{
    if(value-0.01 < 0.001) /// value <= 0.01
        safeMargin = 0.1;
    if(value-0.33 > 0.001) /// value>=0.33
        safeMargin = 0.33;
    else
        safeMargin = value;

    maxX = scene->getWidth() * safeMargin;
    maxY = scene->getHeight() * safeMargin;
    maxWidth = scene->getWidth() * (1.0 - safeMargin);
    maxHeight = scene->getHeight() * (1.0 - safeMargin);
    Log::print(INFO,"Camera:: maxWidth: "+to_string(maxWidth)+" maxHeight: "+to_string(maxHeight));
}

unsigned int Camera::getX() const
{
    return x;
}

void Camera::setX(unsigned int value)
{
    if((maxX < value and x>value) xor (value+getWidth() < maxWidth and x<value))
        x = value;
}

unsigned int Camera::getY() const
{

        return y;
}

void Camera::setY(unsigned int value)
{
    if((maxY < value and y>value) xor (value+getHeight() < maxWidth and y<value))
        y = value;
}

unsigned int Camera::getWidth() const
{
    return width;
}

void Camera::setWidth(unsigned int value)
{
    if((realWidth*0.25 < value and getWidth()>value) xor (value+getX() < maxWidth and getWidth()<value))
        width = value;
}

unsigned int Camera::getHeight() const
{
    return height;
}

void Camera::setHeight(unsigned int value)
{
    if((realHeight*0.25 < value and getHeight()>value) xor (value+getY() < maxHeight and getHeight()<value))
        height = value;
}

void Camera::friction(double &vel)
{
    if(vel > fric)
        vel-=fric;
    else
        if(vel < -fric)
            vel+=fric;
    else
        if(fabs(vel-fric)<fric+eps)
            vel=0.0;
}

void Camera::calcFrameSize()
{
    height = height - 2.0 * 9.0 * vz;
    width = width - 2.0 * 16.0 * vz;
    x = x + 16.0 * vz;
    y = y + 9.0 * vz;
}
void Camera::cutFrame()
{
    Mat m = frameIn.getPixels().clone();
    Rect r(x,y,width,height);
    Mat m2 = m(r).clone();
    frameOut.setPixels(m2);
    frameOut.setDeadline_us(frameIn.getDeadline_us());
    frameOut.setNum(frameIn.getNum());
    Log::print(DEBUG,string(__FUNCTION__)+to_string(frameOut.getPixels().empty()));
}

Camera::~Camera()
{
    for(size_t i=0;i<port.size();i++){
        Log::print(INFO,"\tcamera #"+to_string(id)+" deleting port "+to_string(i) );
        delete port[i];
    }
}

void Camera::moveUp()

{
    cout<<__FUNCTION__<<endl;
    vy-=1.0;
}

void Camera::moveDown()
{
    cout<<__FUNCTION__<<endl;
    vy+=1.0;
}

void Camera::moveLeft()
{
    cout<<__FUNCTION__<<endl;
    vx-=1.0;
}

void Camera::moveRight()
{
    cout<<__FUNCTION__<<endl;
    vx+=1.0;
}

void Camera::zoomIn()
{
    cout<<__FUNCTION__<<endl;
    vz+=1.0;
}

void Camera::zoomOut()
{
    cout<<__FUNCTION__<<endl;
    vz-=1.0;
}

void Camera::sendFrame()
{
//    clog<<"cam "<< id<<" sends frame" << endl;
    Log::print(DEBUG,string(__FUNCTION__)+" camera");
    for(size_t i=0; i<port.size();i++){
        port[i]->sendFrame(&frameOut);
    }
}


