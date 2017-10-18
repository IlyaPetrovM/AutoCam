#include <iostream>
#include "director.h"

using namespace std;

int main()
{
    Scene s("rtsp://camera.ru:1234");
    Director d(s);
    d.work();
    return 0;
}

