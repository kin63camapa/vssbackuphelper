#include "log.h"

bool LOG::LogFileOpen(char *logfile)
{
     this->logfile.open(logfile,std::ios_base::app);
     error=errno;
     return bool(this->logfile.is_open());
}

void LOG::close()
{
    this->logfile.close();
}
