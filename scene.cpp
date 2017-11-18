#include "scene.h"



std::string Scene::getSource() const
{
    return path;
}

int Scene::setSource(const string &value)
{
    path = value;
    if(capture.open(path)){
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


void Scene::getFrame(Mat &_frame)
{
    frameMtx.lock();
    if (!que.empty()) {
        _frame=que.front();
        que.pop();
        Log::print(INFO,string(__FUNCTION__)+" 1.2 que size:"+to_string(que.size()));
    }
    frameMtx.unlock();
}

void Scene::setFrame(const Mat &value)
{
    frameMtx.lock();
    frame = value;
    frameMtx.unlock();
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
//    cout << __FUNCTION__ << endl;
//    Log::print(INFO,string(__FUNCTION__)+" 1");
    if(que.size()<queMaxLen){
        frameMtx.lock();
        capture >> frame;
        que.push(frame);
        Log::print(INFO,string(__FUNCTION__)+" 1.2 que size:"+to_string(que.size()));
        frameMtx.unlock();
    }
//    Log::print(INFO,string(__FUNCTION__)+" 2");
}
