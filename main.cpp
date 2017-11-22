#include <iostream>
#include "director.h"
#include <cstdlib>
#include "log.h"

using namespace std;

int main(int argc,char** argv)
{
    Log logger(DEBUG,"main()");
    Scene s(argv[1],3);
    Director d(s,atoi(argv[2]),atoi(argv[3]));
    d.work();
    return 0;
}

