#ifndef LOG_H
#define LOG_H
#include <fstream>

void printhelp(char *cmd,bool err = false,char error[] = "\n\nERROR PARSING OPTIONS!");
extern bool logMode;

class LOG
{
public:
    std::fstream logfile;
    bool LogFileOpen(char*);
    void close();
    void printError(char * error,bool timestamp = logMode);
    int error;
};

#endif // LOG_H
