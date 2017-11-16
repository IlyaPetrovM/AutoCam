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
    CvWindow(int winWidth,int winHeight, const string _winname, int _flags = WINDOW_AUTOSIZE);
    ~CvWindow();
    void sendFrame(Mat &frame) const;
};

#endif // CVWINDOW_H
