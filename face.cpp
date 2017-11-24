#include "face.h"

Face::Face() : id(faceCnt)
{
    faceCnt++;
}

Face::Face(int _x,int _y,int _w,int _h)
    : id(faceCnt),
     x(_x),y(_y),width(_w),height(_h)
{
    faceCnt++;
}
int Face::getId() const
{
    return id;
}


unsigned int Face::getX() const
{
    return x;
}

void Face::setX(unsigned int value)
{
    x = value;
}

unsigned int Face::getY() const
{
    return y;
}

void Face::setY(unsigned int value)
{
    y = value;
}

unsigned int Face::getWidth() const
{
    return width;
}

void Face::setWidth(unsigned int value)
{
    width = value;
}

unsigned int Face::getHeight() const
{
    return height;
}

void Face::setHeight(unsigned int value)
{
    height = value;
}


Face::~Face()
{
    std::clog<<"   Face #"<<id<<" deleted"<<std::endl;
}
int Face::faceCnt = 0;
