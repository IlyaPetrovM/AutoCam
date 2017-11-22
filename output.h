#ifndef OUTPUT_H
#define OUTPUT_H
#include <opencv2/core.hpp>
#include <iostream>
#include <queue>
#include <mutex>
#include "frame.h"

using namespace cv;
using namespace std;
class Output
{
protected:
    unsigned int frameWidth;
    unsigned int frameHeight;
    queue<Frame> que;
    Mutex queMtx;
public:
    Output(int _frameWidth, int _frameHeight);

    virtual ~Output();
    virtual void sendFrame(Frame *frame) = 0;
};

#endif // OUTPUT_H
