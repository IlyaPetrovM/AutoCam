#include "viewfinder.h"
#include <iostream>

bool ViewFinder::setHeight(float _height)
{
    if(aspect.height <= _height && _height<=scene.height ){
        height = _height;
        return true;
    }else return false;
}
bool ViewFinder::setWidth(float _width)
{
    if(aspect.width <= _width && _width<=scene.width){
        width = _width;
        return true;
    }else return false;
}

float ViewFinder::getWidth() const
{
    return width;
}

float ViewFinder::getHeight() const
{
    return height;
}

float ViewFinder::getY() const
{
    return y;
}

bool ViewFinder::setY(float _y)
{
    if(0<=_y){
        y = _y;
        if((int)(y+height) > scene.height){
            y = (float)scene.height - getHeight();
        }
        return true;
    }else return false;
}

float ViewFinder::getX() const
{
    return x;
}

bool ViewFinder::setX(float _x)
{
    if(0 <= _x){
        x = _x;
        if((int)(x+width) > scene.width){
            x = (float)scene.width - getWidth();
        }
        return true;
    }else return false;
}

void ViewFinder::scale(const float &sc){ /// from center
    setHeight(getHeight()+2.0*aspect.height*sc);
    setWidth(getWidth()+2.0*aspect.width*sc);
    setX(getX() - aspect.width*sc);
    setY(getY() - aspect.height*sc);
    std::clog<<"\t\t\t\t\t"<<getAspect(this->getRect2f().size()).width << "x" << getAspect(this->getRect2f().size()).height<<std::endl;
}

ViewFinder::ViewFinder(Rect scene_) :
    scene(scene_),
    aspect(getAspect(scene_.size())),
    x(scene.x),
    y(scene.y),
    width(scene.width),
    height(scene.height)
{

}
