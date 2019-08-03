#include "detectingrules.h"

DetectingRules::DetectingRules(int _minDist)
    :minDist(_minDist)
{
}

bool DetectingRules::near(Face f1, Face f2)
{
    std::clog << "\t Face #" << f1.getId() << " " << f1.getX() << " vs" << std::endl
              << "\t Face #" << f2.getId() << " " << f2.getX()  << std::endl;
    return (fabs(f1.getX() - f2.getX()) < minDist);
}
