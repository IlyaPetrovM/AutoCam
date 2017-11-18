#ifndef DIRECTOR_H
#define DIRECTOR_H
#include <vector>
#include <iostream>
#include <thread>
#include <exception>

#include "scene.h"
#include "operator.h"
#include "face.h"
#include "cvwindow.h"
#include "rtspserver.h"
#include "detectingrules.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;
using std::chrono::system_clock;

class Director
{
    DetectingRules rules;
    std::vector<Face> facesNew;
    std::vector<Face> faceDub;
    std::vector<size_t> facesHidden;
    std::vector<Operator*> operators;
    bool findDubs();
    bool stopWork;
    void manageOperators();
    void updateScene();
    void help();
    void operatorsList();
    Scene scene;
public:
    Director(Scene s);
    ~Director();
    void findFaces();
    void findHiddenFaces();
    void findNewFaces();
    void work(); // main loop
};

#endif // DIRECTOR_H
