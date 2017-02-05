#ifndef PREVIEW_H
#define PREVIEW_H
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "autocamera.h"
#include "detector.h"
#include <ctime>


#include <iostream>

using namespace cv;
using namespace std;

class Preview
{
    Mat preview;
    Rect2f roiFullSize;
    //drawing
    const float thickness;
    const int dotsRadius;
    const int textOffset;
    const int textThickness;
    const double fontScale;
    const string title;
    Size size;
public:
    Preview(const unsigned int width, const unsigned int height, string windowTitle);

    void show();

    Mat getPreview() const;
    void drawPreview(const Mat &fullFrame, const AutoCamera &cam,
                     const bool bTrackerInitialized, const Rect &focus,
                     const Rect &aim,
                     Detector &det,
                     const int &detWarning, const int &aimCheckerPer, const unsigned int frameCounter);
private:
    Rect median(const vector<Rect> &r);
    void drawThirds(Mat &img, const Rect2f &r, Scalar color = Scalar(0,255,0), const double &dotsRadius = 1);
    void drawRects(Mat &img, vector<Rect> rects, string t = "rect", Scalar color = Scalar(255,0,0), float fontScale = 1.0, float textThickness = 1.0, int textOffset = 0, int thickness = 1, int fontFace = CV_FONT_NORMAL);
};

#endif // PREVIEW_H
