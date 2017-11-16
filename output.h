#ifndef OUTPUT_H
#define OUTPUT_H
#include <opencv2/core.hpp>
#include <iostream>

using namespace cv;

class Output
{
protected:
    unsigned int frameWidth;
    unsigned int frameHeight;

public:
    Output(int _frameWidth, int _frameHeight);

    virtual ~Output();
    virtual void sendFrame(Mat &frame) const = 0;
};

#endif // OUTPUT_H
