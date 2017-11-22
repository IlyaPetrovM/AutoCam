#ifndef FRAME_H
#define FRAME_H
#include <string>
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include "log.h"
//#include <opencv2/highgui.hpp>
using namespace std;
using namespace std::chrono;
using std::chrono::system_clock;
using namespace cv;
class Frame
{
    Mat pixels;
    unsigned long deadline_us;
    static unsigned long lastDeletedFrameNum;
    unsigned long num;
public:
    Frame();
    ~Frame();
    bool cameOnTime();
    void calculateDeadline(unsigned long fps);
    Mat getPixels() const;
    void setPixels(const Mat &value);
    static void setFps(unsigned long value);
    void setNum(unsigned long value);
    unsigned long getNum() const;
    unsigned long getDeadline_us() const;
    void setDeadline_us(unsigned long value);
    void drop(){Log::print(WARN,string("Frame ")+to_string(num)+string(" dropped"));}
};

#endif // FRAME_H
