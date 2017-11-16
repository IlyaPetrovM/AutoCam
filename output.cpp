#include "output.h"

Output::Output(int _frameWidth, int _frameHeight)
    :frameWidth(_frameWidth),frameHeight(_frameHeight)
{

}

Output::~Output()
{
    std::clog << "\tOut Port deleted" << std::endl;
}


