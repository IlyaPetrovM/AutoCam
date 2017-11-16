#ifndef SCENE_H
#define SCENE_H
#include <iostream>
#include <stdlib.h>
#include <string>
#include <opencv2/videoio.hpp>
#include <atomic>
#include <mutex>
#include "log.h"

using namespace std;
using namespace cv;
class Scene
{
    string path;
    VideoCapture capture;
    Mutex frameMtx;
    Mat frame;
public:
    Scene(string _source);
    ~Scene();
    void update();
    std::string getSource() const;
    int setSource(const string &value);

    void getFrame(Mat &_frame);
    void setFrame(const Mat &value);
    int getFps() const;
    int getHeight() const;
    int getWidth() const;
    int getChannels() const;

};

#endif // SCENE_H
