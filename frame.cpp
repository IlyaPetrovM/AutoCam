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



Frame::Frame()
{

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
    pixels = value;
}


void Frame::calculateDeadline(unsigned long fps)
{
    deadline_us = (unsigned long)(cvGetTickCount()/cvGetTickFrequency())+(unsigned long)(1.0*1000000.0/(double)fps);
}

Frame::~Frame()
{
    Log::print(DEBUG,string("Frame ")+to_string(num)+string(" deleted"));
}

bool Frame::cameOnTime()
{
    unsigned long curTime = (cvGetTickCount()/cvGetTickFrequency());
    Log::print(DEBUG,string(" Frame ")+to_string(num)
               +string("\tCurrent time: \t")+
               to_string(curTime)+string(" \n\t\t\t\t\tdeadline\t")+to_string(deadline_us));
//    if(curTime >= deadline_us){
//    if(lastDeletedFrameNum+12<num){
//        lastDeletedFrameNum=num;
//        return false;
//    }}
//    else{
        return true;
//    }
}
