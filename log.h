#ifndef LOG_H
#define LOG_H
#include <string>
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
using namespace std;
using namespace std::chrono;
using std::chrono::system_clock;
typedef enum {DEBUG,INFO,WARN,ERROR,NO} loglevel;
class Log
{
    static loglevel level;
    static string context;
public:
    Log(loglevel _level, string _context);
    static void print(loglevel _level, string msg);
//    static void printTime() const;
};
//Log logger(ERROR);
#endif // LOG_H
