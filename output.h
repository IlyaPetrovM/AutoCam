#ifndef OUTPUT_H
#define OUTPUT_H
#include <opencv2/core.hpp>
#include <iostream>
using namespace cv;

class Output
{

public:
    Output();
    virtual void sendFrame(Mat &frame) const = 0;
};

#endif // OUTPUT_H
