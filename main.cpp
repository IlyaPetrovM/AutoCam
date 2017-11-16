#include <iostream>
#include "director.h"
#include <cstdlib>
#include "log.h"

using namespace std;

int main(int argc,char** argv)
{
    Log logger(WARN,"main()");
    Scene s(argv[1]);
    Director d(s);
    d.work();
    return 0;
}

