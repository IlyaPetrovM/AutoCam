#include "camera.h"

Camera::Camera(Scene *_scene, double _safeMargin, double _friction)
    : id(camCnt),
      scene(_scene)

{
    camCnt++;
    setFric(_friction);
    setSafeMargin(_safeMargin);
    vz=0;
    vx=0;
    vy=0;
    z=minZoom;
    x=maxX*2;
    y=maxY*2;
    width=maxWidth/2;
    height=maxHeight/2;
}

void Camera::update()
{
    x+=vx;
    y+=vy;
    z+=(0.001*vz);
    friction(vx);
    friction(vy);
    friction(vz);

//    calcFrameSize();

    bool wasFrame=scene->getFrame(&frameIn);
    if(frameIn.cameOnTime() && wasFrame){
        cutFrame();
        sendFrame();
    }else{
        frameIn.drop();
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

    maxZoom = 2.0;
    minZoom = 1.0;
    maxX = scene->getWidth() * safeMargin;
    maxY = scene->getHeight() * safeMargin;
    maxWidth = scene->getWidth() * (1.0 - safeMargin);
    maxHeight = scene->getHeight() * (1.0 - safeMargin);
    Log::print(INFO,"Camera:: maxWidth: "+to_string(maxWidth)+" maxHeight: "+to_string(maxHeight));
}

void Camera::friction(double &vel)
{
    if(vel>0.0)vel-=fric;
    else
        if(vel<0.0)vel+=fric;
}

void Camera::calcFrameSize()
{
    height = ((double)height)*(z-1.0);
    width = ((double)width)*(z-1.0);
//    x = ((double)x)+z;
//    y = ((double)y)+z;
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
    if(y > maxY)
        vy++;
    else
        vy=0.0;
}

void Camera::moveDown()
{
    cout<<__FUNCTION__<<endl;
    if(y+height < maxHeight)
        vy--;
    else
        vy=0.0;
}

void Camera::moveLeft()
{
    cout<<__FUNCTION__<<endl;
    if(x > maxX)
        vx--;
    else
        vx=0;
}

void Camera::moveRight()
{
    cout<<__FUNCTION__<<endl;
    if(x+width < maxWidth)
        vx++;
    else
        vx=0;
}

void Camera::zoomIn()
{
    cout<<__FUNCTION__<<endl;
    if(z<maxZoom)
        vz++;
    else
        vz=0;
}

void Camera::zoomOut()
{
    cout<<__FUNCTION__<<endl;
    if(z>minZoom)
        vz--;
    else
        vz=0;
}

void Camera::sendFrame()
{
//    clog<<"cam "<< id<<" sends frame" << endl;
    Log::print(DEBUG,string(__FUNCTION__)+" camera");
    for(size_t i=0; i<port.size();i++){
        port[i]->sendFrame(&frameOut);
    }
}


