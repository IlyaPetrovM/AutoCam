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
    static bool dropFrames;
    static int dropEvery;
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
    void drop(const char *func){Log::print(WARN,string(func)+string("drop Frame ")+to_string(num)+string("\n\tCurrent time: \t")+
                           to_string((cvGetTickCount()/cvGetTickFrequency()))+string(" \n\tdeadline:\t")+to_string(deadline_us)+"\n");}
    static bool getDropFrames();
    static void setDropFrames(bool value);
    static int getDropEvery();
    static void setDropEvery(int value);
};

#endif // FRAME_H
