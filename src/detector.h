#ifndef DETECTOR_H
#define DETECTOR_H
#include "opencv2/core.hpp"
#include "opencv2/objdetect.hpp"
#include <string>
#include "opencv2/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;
class Detector
{
    int frameCounter;
    int faceDetectPer;
    int aimUpdatePer;
    bool bFoundSomeFaces;
    bool bAimDetected;


    const Size smallImgSize;
    CascadeClassifier cascadeFull,cascadeProf; ///< Каскады Хаара для детекции лица
    const int minNeighbors;
    const double scaleFactor;
    const Size minfaceSize;
    Rect aim;

    Mat smallImg;
    Mat graySmall;

    vector<Rect> facesFull,facesProf,faceBuf;
    /**
     * @brief Медиана набора прямоугольников
     * @param[in] r Массив прямоугольников
     * @return Прямоугольник, высота которого - медиана высот, а координаты - медианы координат прямоугольников
     */
    Rect median(const vector<Rect>& r);
public:
    Detector(string cascadeFullName_ ,
             string cascadeProfName_,
             Size smallImgSize_,
             int aimUpdatePer_=1,
             int faceDetectPer_=1,
             int minNeighbors_=1,
             int minFaceHeight_=5,
             double scaleFactor_=1.1);

    const Rect& detect(const Mat& fullFrame);
    const Rect& getAim(){
        return aim;
    }
    const Size& getImgSize(){
        return smallImgSize;
    }
    const vector<Rect>& getFacesFull(){
        return facesFull;
    }
    const vector<Rect>& getFacesProf(){
        return facesProf;
    }
    bool foundFaces(){
        return bFoundSomeFaces;
    }
    bool aimDetected(){
        return bAimDetected;
    }
    void resetAim(){
        aim = Rect(Point(0,0),smallImgSize);
        bAimDetected=false;
    }
};

#endif // DETECTOR_H
