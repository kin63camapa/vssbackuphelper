#include <windows.h>
#include <iostream>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <objbase.h>

typedef HRESULT (_stdcall *CoInitialize___)(LPVOID);
typedef HRESULT (_stdcall *CVBC)(IVssBackupComponents**);

#define PROGVER "This test buil has no version.\n"
extern bool logMode = false;
extern bool rawMode = false;
extern bool compMode = false;
extern bool serviceMode = false;
extern bool serviceForce = false;
extern bool removeOld = false;

void printhelp(char *cmd,bool err = false,char error[] = "ERROR PARSING OPTIONS!\n")
{
    char m[] = " [D:\\] [-option] [VALUE]\n"
               "where D:\\ - destination volume (default C:\\) this argument must be first or not specified!\n"
               "Options:            Values:\n"
               "-h|-help            Show this help.\n\n"
               "-v|-ver|-version    Show version and exit.\n\n"
            //"-t|-type            FULL(dafault)|INCREMENTAL|DIFFERENTIAL|LOG|COPY\n"
            //"                    Processing a shadow copy with the specified type.\n"
            //"\n"
            //"-c|-context         BACKUP|FILE_SHARE_BACKUP|NAS_ROLLBACK|APP_ROLLBACK|LIENT_ACCESSIBLE(dafault)\n"
            //"                    |CLIENT_ACCESSIBLE_WRITERS Processing a shadow copy with the specified context.\n"
            //"\n"
            //"-r|-remove-olders   Remove all shadow copies for the destination volume (exclude current)\n"
            //"                    after the snapshot was successfully created. This option has no values.\n"
            //"\n"
            //"-d|-dry-run         analog -context FILE_SHARE_BACKUP, This option has no values. Incompatible with -context.\n"
            //"\n"
            //"-l|-log             With no value - do not use error stream for error messages, write it to log file.\n"
            //"                    [PATH] - save log to specified log file (default path is ./vssadmin.log).\n"
            //"\n"
            //"-s|-services        With no value - Try to start windows vss services befor backup.\n"
            //"                    force-start - Try to start windows vss services befor backup, even if the service is disabled.\n"
            //"\n"
            //"-raw                Take out to the output stream the contents of the shadow copy instead of the device line\n"
            //"                    [PATH] - save raw out (image) specified file.\n"
            //"\n"
            //"-component-mode     [ENABLE|DISABLE] Processing a shadow copy in component mode.\n"
            ;
    if (err) std::cerr<<error<<"Usage: "<<cmd<<m;
    else std::cout<<"This program makes a shadow copy of the specified volume and returns the device's string for the shadow copy.\nUsage: "<<cmd<<m;
}

int main(int argc, char* argv[])
{
    char drive = 'C';
    bool xp = false;
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
            printhelp(argv[0]);
            return 0;
        }
        else if (!strcmp(argv[1],"--version")
                 ||!strcmp(argv[1],"-version")
                 ||!strcmp(argv[1],"--v")
                 ||!strcmp(argv[1],"-v")
                 ||!strcmp(argv[1],"-ver")
                 ||!strcmp(argv[1],"--ver")
                 )
        {
            std::cout << PROGVER;
        }
        else if (strlen(argv[1])==3)
        {
            if(argv[1][1]==':'&&argv[1][2]=='\\')
            {
                char L=argv[1][0];//for simpe read code only
                if (L>'A'&&L<'Z'||L>'a'&&L<'z')
                    drive = L;
            }//else{printhelp(argv[0],1);return 1;}
        }
        for (int i = 1;i < argc-1;i++)
        {
            if (!strcmp(argv[1],"--version")||!strcmp(argv[1],"--version")){;}
        }
        //else{printhelp(argv[0],1);return 1;}
    }
    TCHAR vol[] = {drive,':','\\','\0'};
    HMODULE ole32dll = LoadLibrary(TEXT("ole32.dll"));
    if (!ole32dll)
    {
        std::cerr << "Error! ole32.dll can not load.\n" << "Errcode is " << GetLastError() << "\n";
        return -1;
    }
    HRESULT result;
    HMODULE vssapidll = LoadLibrary(TEXT("Vssapi.dll"));
    if (vssapidll)
    {
        IVssBackupComponents *backupComponents;
        CVBC cvbc = (CVBC)GetProcAddress(vssapidll, "CreateVssBackupComponentsInternal");
        if(!cvbc)
        {
            cvbc = (CVBC)GetProcAddress(vssapidll, "?CreateVssBackupComponents@@YGJPAPAVIVssBackupComponents@@@Z");
            xp = true;

        }
        CoInitialize___ CoInitialize = (CoInitialize___)GetProcAddress(ole32dll, "CoInitialize");
        if (!CoInitialize)
        {
            std::cerr << "Error! Can not acces ole32.dll::CoInitialize()\n";
            return -1;
        }
        if (!SUCCEEDED(result = (CoInitialize)(0)))
        {
            std::cerr << "Error call ole32.dll::CoInitialize()! ";
            if (result == S_FALSE)
                std::cerr << "The COM library is already initialized on this thread.";
            if (result == RPC_E_CHANGED_MODE)
                std::cerr << "A previous call to CoInitializeEx specified the concurrency model for this thread as multithread apartment (MTA). This could also indicate that a change from neutral-threaded apartment to single-threaded apartment has occurred.";
            if (result == E_ACCESSDENIED)
                std::cerr << "CoInitialize caller does not have sufficient privileges or is not an administrator.";
            if (result == E_OUTOFMEMORY)
                std::cerr << "CoInitialize caller is out of memory or other system resources.";
            if (result == E_INVALIDARG)
                std::cerr << "CoInitialize parameter is incorrect.";
            std::cerr << "\n";
            return -1;
        }
        if (cvbc)
        {
            if (!SUCCEEDED((cvbc)(&backupComponents)))
            {
                std::cerr << "Error in CreateVssBackupComponents! Are u Administrator?\n";
                return -1;
            }
            VSS_ID snapshotSetId;
            if (!SUCCEEDED(result = backupComponents->InitializeForBackup()))
            {
                std::cerr << "Error in InitializeForBackup(), are u running native achitecture (x86 of x64)?\n";
                if (result == S_OK) std::cerr << "Successfully initialized the specified document for backup.";
                if (result == E_ACCESSDENIED)
                    std::cerr << "The caller does not have sufficient backup privileges or is not an administrator.";
                if (result == E_OUTOFMEMORY)
                    std::cerr << "The caller is out of memory or other system resources.";
                if (result == VSS_E_BAD_STATE)
                    std::cerr << "The backup components object is not initialized, this method has been called during a restore operation, or this method has not been called within the correct sequence.";
                if (result == VSS_E_INVALID_XML_DOCUMENT)
                    std::cerr << "The XML document is not valid. Check the event log for details. For more information, see Event and Error Handling Under VSS.";
                /*if (result == VSS_E_UNEXPECTED)
                std::cerr << "Unexpected error. The error code is logged in the error log file. For more information, see Event and Error Handling Under VSS.";*/
                std::cerr << "\n";
                return -1;
            }
            if (!SUCCEEDED(backupComponents->SetBackupState(FALSE, drive == 'C', VSS_BT_FULL)))
            {
                std::cerr << "Error in backupComponents->SetBackupState!\n";
                return -1;
            }
            if (!xp){
                if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_CLIENT_ACCESSIBLE)))
                {
                    if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_BACKUP)))
                    {
                        if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_CLIENT_ACCESSIBLE_WRITERS)))
                        {
                            if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_APP_ROLLBACK)))
                            {
                                std::cerr << "Error in backupComponents->SetContext!\n";
                                return -1;
                            }
                        }
                    }
                }}
            VSS_ID snapshotId;
            if (!SUCCEEDED(result = backupComponents->StartSnapshotSet(&snapshotSetId)))
            {
                std::cerr << "Error in backupComponents->StartSnapshotSet!\n";
                if (result == E_INVALIDARG)std::cerr << "One of the parameter values is not valid.";
                if (result == VSS_E_SNAPSHOT_SET_IN_PROGRESS)std::cerr << "The creation of a shadow copy is in progress, and only one shadow copy creation operation can be in progress at one time. Either wait to try again or return with a failure error code.";
                if (result == E_OUTOFMEMORY)std::cerr << "The caller is out of memory or other system resources.";
                if (result == VSS_E_BAD_STATE)std::cerr << "The backup components object is not initialized, this method has been called during a restore operation, or this method has not been called within the correct sequence.";
                std::cerr << "\n";
                return -1;
            }
            if (!SUCCEEDED(result = backupComponents->AddToSnapshotSet(vol, GUID_NULL, &snapshotId)))
            {

                std::cerr << "Error in AddToSnapshotSet()\n";
                if (result == E_ACCESSDENIED)
                    std::cerr << "The caller does not have sufficient backup privileges or is not an administrator.";
                if (result == E_INVALIDARG)
                    std::cerr << "One of the parameter values is not valid";
                if (result == VSS_E_BAD_STATE)
                    std::cerr << "The backup components object is not initialized, this method has been called during a restore operation, or this method has not been called within the correct sequence.";
                if (result == VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED)
                    std::cerr << "The maximum number of volumes or remote file shares have been added to the shadow copy set. The specified volume or remote file share was not added to the shadow copy set.";
                if (result == VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED)
                    std::cerr << "The volume or remote file share has been added to the maximum number of shadow copy sets. The specified volume or remote file share was not added to the shadow copy set.";
                if (result == E_OUTOFMEMORY)
                    std::cerr << "The caller is out of memory or other system resources.";
                if (result == 0x8004232CL)
                    std::cerr << "The specified volume is nested too deeply to participate in the VSS operation. Possible reasons for this error include the following:\n"
                                 "Trying to create a shadow copy of a volume that resides on a VHD that is contained in another VHD.\n"
                                 "Trying to create a shadow copy of a VHD volume when the volume that contains the VHD is also in the same shadow copy set.";
                if (result == VSS_E_OBJECT_NOT_FOUND)
                    std::cerr << "pwszVolumeName does not correspond to an existing volume or remote file share";
                if (result == VSS_E_PROVIDER_NOT_REGISTERED)
                    std::cerr << "ProviderId does not correspond to a registered provider.";
                if (result == VSS_E_PROVIDER_VETO)
                    std::cerr << "Expected provider error. The provider logged the error in the event log. For more information, see Event and Error Handling Under VSS.";
                if (result == VSS_E_SNAPSHOT_SET_IN_PROGRESS)
                    std::cerr << "Another shadow copy creation is already in progress. Occurs when adding a CSV volume to a snapshot set from multiple nodes at the same time, or while adding a scale out share to the snapshot set from multiple SMB client nodes at the same time.";
                if (result == VSS_E_VOLUME_NOT_SUPPORTED)
                    std::cerr << "The value of the ProviderId parameter is GUID_NULL and no VSS provider indicates that it supports the specified volume or remote file share.";
                if (result == VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER)
                    std::cerr << "The volume or remote file share is not supported by the specified provider.";
                if (result == VSS_E_UNEXPECTED_PROVIDER_ERROR)
                    std::cerr << "The provider returned an unexpected error code. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.";
                std::cerr << "\n";
                return -1;
            }
            IVssAsync *async;
            if (!SUCCEEDED(result = backupComponents->DoSnapshotSet(&async)))
            {
                std::cerr << "Error in backupComponents->DoSnapshotSet!\n";
                /*
E_ACCESSDENIED



The caller does not have sufficient backup privileges or is not an administrator.

E_INVALIDARG



ppAsyncdoes not point to a valid pointer; that is, it is NULL.

E_OUTOFMEMORY



The caller is out of memory or other system resources.

VSS_E_BAD_STATE



The backup components object has not been initialized or the prerequisite calls for a given shadow copy context have not been made prior to calling DoSnapshotSet.

VSS_E_INSUFFICIENT_STORAGE



The system or provider has insufficient storage space. If possible delete any old or unnecessary persistent shadow copies and try again. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.

VSS_E_FLUSH_WRITES_TIMEOUT



The system was unable to flush I/O writes. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times.

VSS_E_HOLD_WRITES_TIMEOUT



The system was unable to hold I/O writes. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times.

VSS_E_NESTED_VOLUME_LIMIT



The specified volume is nested too deeply to participate in the VSS operation.

Windows Server 2008, Windows Vista, Windows Server 2003 and Windows XP:  This return code is not supported.

VSS_E_PROVIDER_VETO



The provider was unable to perform the request at this time. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.

VSS_E_REBOOT_REQUIRED



The provider encountered an error that requires the user to restart the computer.

Windows Server 2003 and Windows XP:  This value is not supported.

VSS_E_TRANSACTION_FREEZE_TIMEOUT



The system was unable to freeze the Distributed Transaction Coordinator (DTC) or the Kernel Transaction Manager (KTM).

Windows Server 2003 and Windows XP:  This value is not supported.

VSS_E_TRANSACTION_THAW_TIMEOUT



The system was unable to thaw the Distributed Transaction Coordinator (DTC) or the Kernel Transaction Manager (KTM).

Windows Server 2003 and Windows XP:  This value is not supported.

VSS_E_UNEXPECTED



Unexpected error. The error code is logged in the error log file. For more information, see Event and Error Handling Under VSS.

Windows Server 2008, Windows Vista, Windows Server 2003 and Windows XP:  This value is not supported until Windows Server 2008 R2 and Windows 7. E_UNEXPECTED is used instead.

VSS_E_UNEXPECTED_PROVIDER_ERROR



The provider returned an unexpected error code. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.
                */
                return -1;
            }
            result = async->Wait();
            async->Release();

            if (!SUCCEEDED(result))
            {
                std::cerr << "Error wait for snapshot create!\n";
                return -1;
            }
            VSS_SNAPSHOT_PROP prop;
            result = backupComponents->GetSnapshotProperties(snapshotId, &prop);
            if (!SUCCEEDED(result))
            {
                std::cerr << "Handle error in GetSnapshotProperties!\n";
                return -1;
            }
            std::wcout << prop.m_pwszSnapshotDeviceObject << "\n";
            backupComponents->Release();
        }
        else
        {
            std::cerr << "Error! Can not acces Vssapi.dll::CreateVssBackupComponentsInternal()\n";
            return -1;
        }
        FreeLibrary(vssapidll);
        FreeLibrary(ole32dll);
    }
    else
    {
        std::cerr << "Error! Vssapi.dll can not load.\n";
        return -1;
    }
    return 0;
}
