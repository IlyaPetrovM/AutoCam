#include <iostream>
#include "director.h"
#include <cstdlib>
#include "log.h"

using namespace std;

int main(int argc,char** argv)
{

    Log logger(DEBUG,"main()");
    if(argc<5){
        cout << __VERSION__ << __DATE__ <<__TIME__ <<"Usage: [path to media] [target width] [target height] [drop frames{0,1}] [drop every N frame{>0}]"<<endl;
        return -1;
    }
    Frame::setDropFrames(atoi(argv[4]));
    Frame::setDropEvery(atoi(argv[5]));
    Scene s(argv[1],3);
    Director d(s,atoi(argv[2]),atoi(argv[3]));
    d.work();
    return 0;
}

