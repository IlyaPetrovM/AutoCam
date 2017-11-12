#include <iostream>
#include "director.h"

using namespace std;

int main(int argc,char** argv)
{
    Scene s(argv[1]);
    Director d(s);
    d.work();
    return 0;
}

