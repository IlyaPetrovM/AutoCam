#include "scene.h"



std::string Scene::getSource() const
{
    return path;
}

int Scene::setSource(const string &value)
{
    path = value;
    if(capture.open(path)){
        frameCnt=0;
        update();
        Log::print(WARN,path+" opened");
        cout << "Video params:" <<
                "\n\tfps:"<< getFps() <<
                "\n\t"<<getWidth()<<"x"<<getHeight()<<
                "\n\tFOURCC:"<<capture.get(CAP_PROP_FOURCC)<<
                "\n\tFORMAT:"<<capture.get(CAP_PROP_FORMAT)<<
                "\n\tMODE:"<<capture.get(CAP_PROP_MODE)<<
                "\n\tCONVERT_RGB:"<<capture.get(CAP_PROP_CONVERT_RGB)<<endl;
    }else{
        cerr<<"Unable to open "<<path<<endl;
    }
}


bool Scene::getFrame(Frame *_frame)
{
    bool wasFrame=false;
    frameMtx.lock();
    if (!que.empty()) {
        *_frame=que.front();
        que.pop();
        wasFrame=true;
    }
    frameMtx.unlock();
    return wasFrame;
}

int Scene::getFps() const
{
    return capture.get(CAP_PROP_FPS);
}

inline int Scene::getHeight() const
{
    return capture.get(CAP_PROP_FRAME_HEIGHT);
}

inline int Scene::getWidth() const
{
    return capture.get(CAP_PROP_FRAME_WIDTH);
}

int Scene::getChannels() const
{
    return capture.get(CAP_PROP_FORMAT);
}
unsigned long Scene::getFrameCnt() const
{
    return frameCnt;
}

Scene::Scene(std::string _source, unsigned int _queMaxLen)
    :path(_source), queMaxLen(_queMaxLen)
{
    setSource(path);

}

Scene::~Scene()
{

}

void Scene::update()
{
    frameMtx.lock();
    if(que.size()<=queMaxLen){
        frame.calculateDeadline(getFps());
        Mat tempFrame;
        capture >> tempFrame;
        frame.setNum(frameCnt++);
        if(frame.cameOnTime() && (!tempFrame.empty())){
            frame.setPixels(tempFrame);
            que.push(frame);
            Log::print(DEBUG,string(__FUNCTION__)+" pixels in Frame:"+to_string(que.back().getDeadline_us()));
        }else{
            frame.drop(__FUNCTION__);
        }
    }
    frameMtx.unlock();
}
