#ifndef SCENE_H
#define SCENE_H
#include <iostream>
#include <stdlib.h>
#include <string>
#include <opencv2/videoio.hpp>
#include <atomic>
#include <mutex>
#include "log.h"
#include <queue>
#include "frame.h"

using namespace std;
using namespace cv;
class Scene
{
    string path;
    VideoCapture capture;
    Mutex frameMtx;
    Frame frame;
    queue<Frame> que;
    unsigned int queMaxLen;
    unsigned long frameCnt;
public:
    Scene(string _source, unsigned int _queMaxLen=25);
    ~Scene();
    void update();
    std::string getSource() const;
    int setSource(const string &value);

    void getFrame(Frame *_frame);
    int getFps() const;
    int getHeight() const;
    int getWidth() const;
    int getChannels() const;

    unsigned long getFrameCnt() const;
};

#endif // SCENE_H
