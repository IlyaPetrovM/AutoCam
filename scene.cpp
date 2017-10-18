#include "scene.h"



std::string Scene::getSource() const
{
    return source;
}

void Scene::setSource(const std::string &value)
{
    source = value;
}

Scene::Scene(std::string _source)
    :source(_source)
{

}

void Scene::update()
{
    //    std::cout << "Scene updated." << std::endl;
}
