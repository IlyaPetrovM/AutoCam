#include "log.h"

Log::Log(loglevel _level, string _context)
{
    if(level==NO){
        level = _level;
        context = _context;
        clog << " tick freq is"<< cvGetTickFrequency() << endl;
    }else {
        clog << "Logger level was already set to "<< level << " in "<< context <<endl;
    }
}

loglevel Log::level = NO;
string Log::context = "";

void Log::print(loglevel _level, string msg)
{
    if(_level<=level){
        clog << ""<< cvGetTickCount()/cvGetTickFrequency() << "\t" <<_level<< "\t"<< msg << endl;
    }
}
