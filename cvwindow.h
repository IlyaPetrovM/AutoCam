#ifndef CVWINDOW_H
#define CVWINDOW_H
#include "output.h"
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;
class CvWindow : public Output
{
    String winname;
public:
    CvWindow(int winWidth,int winHeight, const string _winname, int _flags = 0);
    ~CvWindow();
    void sendFrame(Frame *frame);
};

#endif // CVWINDOW_H
