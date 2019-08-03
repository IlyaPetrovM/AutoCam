#include "output.h"

Output::Output(int _frameWidth, int _frameHeight)
    :targetWidth(_frameWidth),targetHeight(_frameHeight)
{

}

Output::~Output()
{
    std::clog << "\tOut Port deleted" << std::endl;
}


