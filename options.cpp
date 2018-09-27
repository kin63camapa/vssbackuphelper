#include <iostream>
#include "options.h"

bool networkMode = false;
bool rawMode = false;
bool compMode = false;
bool serviceMode = false;
bool serviceForce = false;
bool removeOld = false;
bool errorExit = true;
unsigned char logLevel = 1;
_VSS_BACKUP_TYPE bkpType = VSS_BT_FULL;
_VSS_SNAPSHOT_CONTEXT bkpContext = VSS_CTX_CLIENT_ACCESSIBLE;
char *logfile = NULL;
char *rawfile = NULL;
char drive = 'C';
TCHAR vol[4] = {drive,':','\\','\0'};

bool parseOptions(int argc, char *argv[])
{
    int parsedOpts = NULL;
    if (argc >= 2)
    {
        if (!strcmp(argv[1],"--help")
                ||!strcmp(argv[1],"-help")
                ||!strcmp(argv[1],"--h")
                ||!strcmp(argv[1],"-h")
                ||!strcmp(argv[1],"/?")
                ||!strcmp(argv[1],"-H")
                )
        {
            parsedOpts++;
            printhelp(argv[0]);
            std::exit(EXIT_SUCCESS);
        }
        else if (!strcmp(argv[1],"--version")
                 ||!strcmp(argv[1],"-version")
                 ||!strcmp(argv[1],"--v")
                 ||!strcmp(argv[1],"-v")
                 ||!strcmp(argv[1],"-ver")
                 ||!strcmp(argv[1],"--ver")
                 )
        {
            parsedOpts++;
            std::cout << PROGVER;
            std::exit(EXIT_SUCCESS);
        }
        else if (strlen(argv[1])==3)
        {
            if(argv[1][1]==':'&&argv[1][2]=='\\')
            {
                char L=argv[1][0];//for simpe read code only
                if (L>'A'&&L<'Z'||L>'a'&&L<'z')
                {
                    drive = L;
                    parsedOpts++;
                }
            }
        }
        for (int i = 1;i < argc;i++)
        {
            if (!strcmp(argv[i],"-log-level"))
            {
                parsedOpts++;
                i++;
                if ((argc > i) && (argv[i][0] != '-'))
                {
                    if (!strcmp(argv[i],"error")||!strcmp(argv[i],"1"))
                    {
                        parsedOpts++;
                        logLevel = 1;
                    }
                    if (!strcmp(argv[i],"warn")||!strcmp(argv[i],"2"))
                    {
                        parsedOpts++;
                        logLevel = 2;
                    }
                    if (!strcmp(argv[i],"info")||!strcmp(argv[i],"3"))
                    {
                        parsedOpts++;
                        logLevel = 3;
                    }
                    if (!strcmp(argv[i],"debug")||!strcmp(argv[i],"4"))
                    {
                        parsedOpts++;
                        logLevel = 4;
                    }
                }
            }
            if (!strcmp(argv[i],"-services")||!strcmp(argv[i],"-s"))
            {
                parsedOpts++;
                serviceMode = true;

                if ((argc > i+1) && (argv[i+1][0] != '-'))
                {
                    if (!strcmp(argv[i+1],"force-start")||!strcmp(argv[i+1],"force"))
                    {
                        parsedOpts++;
                        serviceForce = true;
                    }
                }
            }
            if (!strcmp(argv[i],"-type")||!strcmp(argv[i],"-t"))
            {
                parsedOpts++;
                if ((argc > i+1) && (argv[i+1][0] != '-'))
                {
                    if (!strcmp(argv[i+1],"FULL"))
                    {
                        parsedOpts++;
                        bkpType = VSS_BT_FULL;
                    }
                    if (!strcmp(argv[i+1],"INCREMENTAL"))
                    {
                        parsedOpts++;
                        bkpType = VSS_BT_INCREMENTAL;
                    }
                    if (!strcmp(argv[i+1],"DIFFERENTIAL"))
                    {
                        parsedOpts++;
                        bkpType = VSS_BT_DIFFERENTIAL;
                    }
                    if (!strcmp(argv[i+1],"LOG"))
                    {
                        parsedOpts++;
                        bkpType = VSS_BT_LOG;
                    }
                    if (!strcmp(argv[i+1],"COPY"))
                    {
                        parsedOpts++;
                        bkpType = VSS_BT_COPY;
                    }
                }
            }
            if (!strcmp(argv[i],"-context")||!strcmp(argv[i],"-c"))
            {
                parsedOpts++;
                if ((argc > i+1) && (argv[i+1][0] != '-'))
                {
                    if (!strcmp(argv[i+1],"BACKUP"))
                    {
                        parsedOpts++;
                        bkpContext = VSS_CTX_BACKUP;
                    }
                    if (!strcmp(argv[i+1],"FILE_SHARE_BACKUP"))
                    {
                        parsedOpts++;
                        bkpContext = VSS_CTX_FILE_SHARE_BACKUP;
                    }
                    if (!strcmp(argv[i+1],"NAS_ROLLBACK"))
                    {
                        parsedOpts++;
                        bkpContext = VSS_CTX_NAS_ROLLBACK;
                    }
                    if (!strcmp(argv[i+1],"APP_ROLLBACK"))
                    {
                        parsedOpts++;
                        bkpContext = VSS_CTX_APP_ROLLBACK;
                    }
                    if (!strcmp(argv[i+1],"CLIENT_ACCESSIBLE"))
                    {
                        parsedOpts++;
                        bkpContext = VSS_CTX_CLIENT_ACCESSIBLE;
                    }
                    if (!strcmp(argv[i+1],"CLIENT_ACCESSIBLE_WRITERS"))
                    {
                        parsedOpts++;
                        bkpContext = VSS_CTX_CLIENT_ACCESSIBLE_WRITERS;
                    }
                }
            }
            if (!strcmp(argv[i],"-dry-run")||!strcmp(argv[i],"-d"))
            {
                parsedOpts++;
                bkpContext = VSS_CTX_FILE_SHARE_BACKUP;
            }
            if (!strcmp(argv[i],"-network")||!strcmp(argv[i],"-n"))
            {
                parsedOpts++;
                networkMode = true;
            }
            if (!strcmp(argv[i],"-log")||!strcmp(argv[i],"-l"))
            {
                logMode = true;
                parsedOpts++;
                if ((argc > i+1) && (argv[i+1][0] != '-'))
                {
                    logfile = new char[]=argv[i+1];
                    i++;
                    parsedOpts++;
                }else{logfile = new char[]=LOGFILE;}
            }
            if (!strcmp(argv[i],"-remove-old")||!strcmp(argv[i],"-r"))
            {
                removeOld = true;
                parsedOpts++;
            }
            if (!strcmp(argv[i],"-raw"))
            {
                rawMode = true;
                parsedOpts++;
                if ((argc > i+1) && (argv[i+1][0] != '-'))
                {
                    rawfile = new char[] = argv[i+1];
                    i++;
                    parsedOpts++;
                }
                else rawfile = NULL;
            }
            if (!strcmp(argv[i],"-component-mode"))
            {
                compMode = true;
                parsedOpts++;
            }
        }
    }
    if (logMode && !logfile) logfile = new char[]=LOGFILE;
    if (logMode)
    {
        if (!log.LogFileOpen(logfile))
        {
            char *err = new char[28+strlen(logfile)+strlen(strerror(log.error))];
            strcpy(err,"ERROR: Can not open the log file: ");
            strcat(err,logfile);
            strcat(err," ");
            strcat(err,strerror(log.error));
            printhelp(argv[0],1,err);
            std::exit(EXIT_FAILURE);
        }
    }
    vol[0] = drive;

    if (parsedOpts == argc-1)
    {
        if (networkMode&&!rawMode&&logLevel > 1)log.printError("WARNING: Option -network was specified without -raw, ignoring!");
        return true;
    }
    printhelp(argv[0],1);
    return false;
}
