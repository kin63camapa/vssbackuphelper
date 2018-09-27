#ifndef OPTIONS_H
#define OPTIONS_H

#include <vss.h>
#include <log.h>
#define PRODVER_
#define PRODVER "0.0.0.4"
#define PROGVER "VSSBackupHelper version"PRODVER"\n"
#define LOGFILE "./vssadmin.log"
//extern bool logMode;//log.h
extern bool networkMode;
extern bool rawMode;
extern bool compMode;
extern bool serviceMode;
extern bool serviceForce;
extern bool removeOld;
extern LOG log;
extern unsigned char logLevel;
extern _VSS_BACKUP_TYPE bkpType;
extern _VSS_SNAPSHOT_CONTEXT bkpContext;
extern char *logfile;
extern char *rawfile;
extern char drive;
extern TCHAR vol[4];

//void SetDefault();
bool parseOptions(int argc,char*argv[]);

#endif // OPTIONS_H
