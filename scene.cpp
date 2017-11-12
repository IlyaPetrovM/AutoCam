#include "scene.h"



std::string Scene::getSource() const
{
    return path;
}

void Scene::setSource(const string &value)
{
    path = value;
    if(capture.open(path)){
        cout<<path<<" opened"<<endl;
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
