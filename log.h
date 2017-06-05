#ifndef LOG_H
#define LOG_H
#include <fstream>

class LOG
{
public:
    std::fstream logfile;
    bool LogFileOpen(char*);
    void close();
    int error;
};

#endif // LOG_H
