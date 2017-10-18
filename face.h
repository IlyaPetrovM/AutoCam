#ifndef FACE_H
#define FACE_H
#include <iostream>
class Face
{
    int id;
    static int faceCnt;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
public:
    Face();
    ~Face();

    int getId() const;
    unsigned int getX() const;
    void setX(unsigned int value);
    unsigned int getY() const;
    void setY(unsigned int value);
    unsigned int getWidth() const;
    void setWidth(unsigned int value);
    unsigned int getHeight() const;
    void setHeight(unsigned int value);
};


#endif // FACE_H
