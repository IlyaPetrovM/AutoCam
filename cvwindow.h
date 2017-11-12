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
    CvWindow(const string _winname, int _flags = WINDOW_NORMAL);
    ~CvWindow();
    void sendFrame(Mat &frame) const;
};

#endif // CVWINDOW_H
