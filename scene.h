#ifndef SCENE_H
#define SCENE_H
#include <iostream>
#include <stdlib.h>
#include <string>
class Scene
{
    std::string source;
public:
    Scene(std::string _source);
    void update();
    std::string getSource() const;
    void setSource(const std::string &value);
};

#endif // SCENE_H
