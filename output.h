#ifndef OUTPUT_H
#define OUTPUT_H
#include <opencv2/core.hpp>
#include <iostream>
#include <queue>

using namespace cv;
using namespace std;
class Output
{
protected:
    unsigned int frameWidth;
    unsigned int frameHeight;
    queue<Mat> que;
public:
    Output(int _frameWidth, int _frameHeight);

    virtual ~Output();
    virtual void sendFrame(const Mat &frame) = 0;
};

#endif // OUTPUT_H
