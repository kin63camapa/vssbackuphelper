#include <iostream>
#include <ctime>
#include "log.h"

bool logMode = false;

bool LOG::LogFileOpen(char *logfile)
{
     this->logfile.open(logfile,std::ios_base::app);
     this->logfile.rdbuf()->pubsetbuf( 0, 0 );
     error=errno;
     return bool(this->logfile.is_open());
}

void LOG::close()
{
    this->logfile.close();
}

void LOG::printError(char * error,bool timestamp)
{
    const time_t ctt = time(0);
    char*tm;
    if (timestamp){
        tm = asctime(localtime(&ctt));
        tm[strlen(tm)-1] = ' ';
    }
    else tm = "";
    if (logMode) logfile << tm << error << std::flush;
    else std::cerr << tm << error << std::flush;
}

void printhelp(char *cmd,bool err,char error[])
{
    char m[] = " [D:\\] [-option] [VALUE]\n"
"where D:\\ - destination volume (default C:\\) this argument must be first or\n"
"not specified!\n\n"
"Options:            Values:\n"
"-h|-help            Show this help and exit. This argument must be one.\n\n"
"-v|-ver|-version    Show version and exit. This argument must be one.\n"
"                    !WARNINIG -v is NOT be verbose! Use -log-level 2|3|4 for\n"
"                    debug out.\n\n"
"-t|-type            FULL(default)|INCREMENTAL|DIFFERENTIAL|LOG|COPY\n"
"                    Processing a shadow copy with the specified type.\n"
"\n"
"-c|-context         BACKUP|FILE_SHARE_BACKUP|NAS_ROLLBACK|APP_ROLLBACK|\n"
"                    CLIENT_ACCESSIBLE(default)|CLIENT_ACCESSIBLE_WRITERS\n"
"                    Processing a shadow copy with the specified context.\n"
"\n"
"-d|-dry-run         analog -context FILE_SHARE_BACKUP, This option has no\n"
"                    values. Incompatible with -context.\n"
"\n"
"-component-mode     Processing a shadow copy in component mode. This option has\n"
"                    no values.\n"
"\n"
"-l|-log             With no value - do not use error stream for error messages,\n"
"                    write it to log file.\n"
"                    [<PATH>] - save log to specified log file (default path is \n"
"                    ./vssadmin.log).\n"
"\n"
"-log-level          1-4 or error|warn|info|debug level of logging. Default \n"
"                    value is 1 or \"error\".\n"
"\n"
"-raw                Take out to the output stream the contents of the shadow\n"
"                    copy instead of the device line\n"
"                    [<PATH>] - save raw out (image) to the specified file.\n"
"\n"
//"-n|-network         In raw mode, try to continue writing to the output file,\n"
//"                    even if errors have occurred.\n"
//"\n"
//"-r|-remove-old      Remove all shadow copies for the destination volume (exclude\n"
//"                    current) after the snapshot was successfully created. This\n"
//"                    option has no values.\n"
//"\n"
//"-s|-services        With no value - Try to start windows vss services befor\n"
//"                    backup.\n"
//"                    force-start|force - Try to start windows vss services befor\n"
//"                    backup, even if the service is disabled.\n"
//"\n"
;
    if (err) std::cerr<<error<<"\nUsage: "<<cmd<<m;
    else std::cout<<"This program makes a shadow copy of the specified volume and returns the device's string for the shadow copy.\nUsage: "<<cmd<<m;
}
