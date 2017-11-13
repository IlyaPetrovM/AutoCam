#include "scene.h"



std::string Scene::getSource() const
{
    return path;
}

void Scene::setSource(const string &value)
{
    path = value;
    if(capture.open(path)){
        cout<<path<<", opened"<<endl;
        cout << "Video params:" <<
                "\n\tfps:"<< getFps() <<
                "\n\t"<<getWidth()<<"x"<<getHeight()<<
                "\n\tFOURCC:"<<capture.get(CAP_PROP_FOURCC)<<
                "\n\tFORMAT:"<<capture.get(CAP_PROP_FORMAT)<<endl;
    }else{
        cerr<<"Unable to open "<<path<<endl;
        return;
    }
}


void Scene::getFrame(Mat &_frame)
{
    frameMtx.lock();
    _frame=frame;
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
Scene::Scene(std::string _source)
    :path(_source)
{
    setSource(path);
}

Scene::~Scene()
{

}

void Scene::update()
{
    frameMtx.lock();
    capture >> frame;
    frameMtx.unlock();
}
