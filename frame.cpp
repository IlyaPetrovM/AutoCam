#include "frame.h"
unsigned long Frame::lastDeletedFrameNum=0;
unsigned long Frame::getDeadline_us() const
{
    return deadline_us;
}

void Frame::setDeadline_us(unsigned long value)
{
    deadline_us = value;
}

bool Frame::getDropFrames()
{
    return dropFrames;
}

void Frame::setDropFrames(bool value)
{
    dropFrames = value;
}

int Frame::getDropEvery()
{
    return dropEvery;
}

void Frame::setDropEvery(int value)
{
    if(value>0){
        dropEvery = value;
    }else{
        Log::print(WARN,string(__FUNCTION__)+"dropEvery parameter shuld be >0. Setting default value: "+to_string(dropEvery));
    }
}

Frame::Frame()
{
    num=-1;
}

unsigned long Frame::getNum() const
{
    return num;
}

void Frame::setNum(unsigned long value)
{
    num = value;
}

Mat Frame::getPixels() const
{
    return pixels;
}

void Frame::setPixels(const Mat &value)
{
    pixels = value.clone();
}
bool Frame::dropFrames=false;
int Frame::dropEvery=1;

void Frame::calculateDeadline(unsigned long fps)
{
    deadline_us = (unsigned long)(cvGetTickCount()/cvGetTickFrequency())+(unsigned long)(1000000.0/(double)fps);
}

Frame::~Frame()
{
    Log::print(DEBUG,string("Frame ")+to_string(num)+string(" deleted"));
}

bool Frame::cameOnTime()
{
    unsigned long curTime = (cvGetTickCount()/cvGetTickFrequency());
//    if(dropFrames){
//        if(curTime >= deadline_us){
////            if(lastDeletedFrameNum+dropEvery<num){
////                lastDeletedFrameNum=num;
//                return false;
////            }
//        }
//        else{
//            return true;
//        }
//    }else{
        return true;
//    }
}
