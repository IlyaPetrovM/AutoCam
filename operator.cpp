#include "operator.h"
int Operator::opCnt=0;

Face Operator::getFace() const
{
    return face;
}

void Operator::setFace(const Face &value)
{
    face = value;
}

int Operator::getId() const
{
    return id;
}
Operator::Operator(Scene *s, Camera *cam, Face *target) :
    scene(s),
    camera(*cam),
    face(*target),
    id(opCnt)
{
    std::clog << " Operator #" << id << " takes camera #" << camera.getId() << " to film the person #" << face.getId() << std::endl;
    opCnt++;
    typeOfWork=MANUAL;
}

Operator::~Operator()
{
    std::clog << "  Operator #" << id << " fired" << std::endl;
}


void Operator::work()
{
    if(typeOfWork==AUTO){
        workAuto();
    }else if(typeOfWork==MANUAL){
        workManual();
    }
}


workType Operator::getTypeOfWork() const
{
    return typeOfWork;
}

void Operator::setTypeOfWork(const workType &value)
{
    typeOfWork = value;
}

void Operator::sendCommand(char _cmd)
{
    cmd=_cmd;
}
void Operator::workManual()
{
    switch (cmd) {
    case 'w':
        camera.moveUp();
        break;
    case 'a':
        camera.moveLeft();
        break;
    case 's':
        camera.moveDown();
        break;
    case 'd':
        camera.moveRight();
        break;
    case 'q':
        camera.zoomOut();
        break;
    case 'e':
        camera.zoomIn();
        break;
    case 'm':
        setTypeOfWork(MANUAL);
        break;
    case 'n':
        setTypeOfWork(AUTO);
        break;
    default:
        break;
    }
    cmd=' ';
    camera.update(); //important
}

void Operator::workAuto()
{
    switch (cmd) {
    case 'm':
        setTypeOfWork(MANUAL);
        break;
    case 'n':
        setTypeOfWork(AUTO);
        break;
    default: break;
    }
    cmd=' ';

    std::clog << "\rOperator #" << id << " work automatically" << std::endl;
    // get new face coordinates in scene
    // compute ideal composition for this face in this scene
    // move the camera
}
