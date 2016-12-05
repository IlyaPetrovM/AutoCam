#include "detector.h"


const Rect &Detector::detect(const Mat& fullFrame)
{
    if(frameCounter%faceDetectPer==0){
        resize( fullFrame, smallImg, smallImgSize, 0, 0, INTER_LINEAR );
        cvtColor( smallImg, graySmall, COLOR_BGR2GRAY );

        /* Поиск лиц в анфас */
        cascadeFull.detectMultiScale(graySmall, facesFull,
                                     scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE,minfaceSize);
        /* Поиск лиц в профиль */
        cascadeProf.detectMultiScale( graySmall, facesProf,
                                      scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE,minfaceSize);
        foundFaces = !(facesFull.empty() && facesProf.empty());
    }else foundFaces = false;

    if(foundFaces){
        if(!facesFull.empty() && ((facesFull[0]&aim).area()>0))faceBuf.push_back(facesFull[0]);
        if(!facesProf.empty() && ((facesProf[0]&aim).area()>0))faceBuf.push_back(facesProf[0]);
    }
    if(frameCounter%aimUpdatePer == 0){
        if(!faceBuf.empty()) {
            aim = median(faceBuf);
            faceBuf.clear();
        }
    }
    return aim;
}


Rect Detector::median(const vector<Rect> &r){
    static vector<int> x,y,h;
    x.resize(r.size());
    y.resize(r.size());
    h.resize(r.size());
    for (int i = 0; i < r.size(); ++i) {
        x[i]=r[i].x;
        y[i]=r[i].y;
        h[i]=r[i].height;
    }
    sort(x.begin(),x.end());
    sort(y.begin(),y.end());
    sort(h.begin(),h.end());
    return Rect(x[x.size()/2],y[y.size()/2],h[h.size()/2],h[h.size()/2]);
}

Detector::Detector(string cascadeFullName_, string cascadeProfName_, Size smallImgSize_, int aimUpdatePer_, int faceDetectPer_, int minNeighbors_, int minFaceHeight_, double scaleFactor_)
    : faceDetectPer(faceDetectPer_),
      smallImgSize(smallImgSize_),
      minNeighbors(minNeighbors_),
      scaleFactor(scaleFactor_),
      minfaceSize(Size(minFaceHeight_,minFaceHeight_)),
      aim(Rect(Point(0,0),smallImgSize_)),
      foundFaces(false),
      frameCounter(0),
      aimUpdatePer(aimUpdatePer_)
{
    if( !cascadeFull.load( cascadeFullName_ ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
    }
    if( !cascadeProf.load( cascadeProfName_ ) )
    {
        cerr << "ERROR: Could not load classifier 2 cascade" << endl;
    }
}
