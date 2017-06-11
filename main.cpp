#include <iostream>
#include <windows.h>
#include <Winternl.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include "log.h"
#include "options.h"

typedef HRESULT (_stdcall *CoInitialize___)(LPVOID);
typedef HRESULT (_stdcall *CVBC)(IVssBackupComponents**);
typedef HRESULT (_stdcall *NtDeviceIoControlFile___)(HANDLE,HANDLE,PIO_APC_ROUTINE,PVOID,PIO_STATUS_BLOCK,ULONG,PVOID,ULONG,PVOID,ULONG);

LOG log;

int main(int argc, char* argv[])
{
    bool xp = false;
    if (!parseOptions(argc,argv))return 9;
    if (logLevel > 3)
    {
        log.printError("DEBUG: End parsing options. Starting ");
        log.printError(PROGVER,0);
    }
    HMODULE ole32dll = LoadLibrary(TEXT("ole32.dll"));
    if (!ole32dll)
    {
        log.printError("Error! ole32.dll can not load. ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: ole32.dll was successfully opened.\n");
    HMODULE vssapidll = LoadLibrary(TEXT("Vssapi.dll"));
    if (!vssapidll)
    {
        log.printError("ERROR: Vssapi.dll can not load. ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: Vssapi.dll was successfully opened.\n");
    HRESULT result;
    IVssBackupComponents *backupComponents;
    CVBC cvbc = (CVBC)GetProcAddress(vssapidll, "CreateVssBackupComponentsInternal");
    if(!cvbc)
    {
        if (logLevel > 1){log.printError("WARNING: Error get API CreateVssBackupComponents. try to ?CreateVssBackupComponents@@YGJPAPAVIVssBackupComponents@@@Z\n");}
        cvbc = (CVBC)GetProcAddress(vssapidll, "?CreateVssBackupComponents@@YGJPAPAVIVssBackupComponents@@@Z");
        if (!cvbc)
        {
            if (logLevel > 1){log.printError("WARNING: Error get API CreateVssBackupComponents. try to ?CreateVssBackupComponents@@YAJPEAPEAVIVssBackupComponents@@@Z\n");}
            cvbc  = (CVBC)GetProcAddress(vssapidll, "?CreateVssBackupComponents@@YAJPEAPEAVIVssBackupComponents@@@Z");
            if (!cvbc)
            {
                log.printError("ERROR: Can not get API CreateVssBackupComponents. ");
                char s[MAX_PATH] = {0};
                if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
                log.printError("\n",0);
                return 1;
            }
        }
        xp = true;
        //temp
        log.printError("ERROR: Windows XP or 2003 is not supported in this version.\n");
        return 0;
    }
    if (logLevel > 3) log.printError("DEBUG: API CreateVssBackupComponents from Vssapi.dll was successfully opened.\n");
    CoInitialize___ CoInitialize = (CoInitialize___)GetProcAddress(ole32dll, "CoInitialize");
    if (!CoInitialize)
    {
        log.printError("Error! Can not acces ole32.dll::CoInitialize()\n");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: CoInitialize api from ole32.dll was successfully loaded.\n");
    if (!SUCCEEDED(result = (CoInitialize)(0)))
    {
        log.printError("Error call ole32.dll::CoInitialize()! ");
        if (result == S_FALSE)
            log.printError("The COM library is already initialized on this thread.");
        if (result == RPC_E_CHANGED_MODE)
            log.printError("A previous call to CoInitializeEx specified the concurrency model for this thread as multithread apartment (MTA). This could also indicate that a change from neutral-threaded apartment to single-threaded apartment has occurred.");
        if (result == E_ACCESSDENIED)
            log.printError("CoInitialize caller does not have sufficient privileges or is not an administrator.");
        if (result == E_OUTOFMEMORY)
            log.printError("CoInitialize caller is out of memory or other system resources.");
        if (result == E_INVALIDARG)
            log.printError("CoInitialize parameter is incorrect.");
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: CoInitialize api from ole32.dll was successfully executed.\n");
    if (!SUCCEEDED((cvbc)(&backupComponents)))
    {
        log.printError("ERROR: Can not execute CreateVssBackupComponents! Are u Administrator? ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: CreateVssBackupComponents api from vssapi.dll was successfully executed.\n");
    VSS_ID snapshotSetId;
    if (!SUCCEEDED(result = backupComponents->InitializeForBackup()))
    {
        log.printError("Error in InitializeForBackup(), are u running native achitecture (x86 of x64)?\n");
        if (result == S_OK) log.printError("Successfully initialized the specified document for backup.");
        if (result == E_ACCESSDENIED)
            log.printError("The caller does not have sufficient backup privileges or is not an administrator.");
        if (result == E_OUTOFMEMORY)
            log.printError("The caller is out of memory or other system resources.");
        if (result == VSS_E_BAD_STATE)
            log.printError("The backup components object is not initialized, this method has been called during a restore operation, or this method has not been called within the correct sequence.");
        if (result == VSS_E_INVALID_XML_DOCUMENT)
            log.printError("The XML document is not valid. Check the event log for details. For more information, see Event and Error Handling Under VSS.");
        /*if (result == VSS_E_UNEXPECTED)
                log.printError("Unexpected error. The error code is logged in the error log file. For more information, see Event and Error Handling Under VSS.");*/
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: InitializeForBackup() was successfully executed.\n");
    if (logLevel > 1&&drive == 'C')log.printError("WARNING: Bootable system state is being performed.\n");
    if (!SUCCEEDED(backupComponents->SetBackupState(compMode, drive == 'C', bkpType)))
    {
        log.printError("ERROR: Error in backupComponents->SetBackupState!\n");
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: SetBackupState() was successfully executed.\n");
    if (!xp)
    {
        if (!SUCCEEDED(backupComponents->SetContext(bkpContext)))
        {
            if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_BACKUP)))
            {
                if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_CLIENT_ACCESSIBLE_WRITERS)))
                {
                    if (!SUCCEEDED(backupComponents->SetContext(VSS_CTX_APP_ROLLBACK)))
                    {
                        log.printError("Error in backupComponents->SetContext! ");
                        char s[MAX_PATH] = {0};
                        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
                        log.printError("\n",0);
                        return -1;
                    }
                }
            }
        }
        if (logLevel > 3) log.printError("DEBUG: SetContext() was successfully executed.\n");
    }
    VSS_ID snapshotId;
    if (!SUCCEEDED(result = backupComponents->StartSnapshotSet(&snapshotSetId)))
    {
        log.printError("Error in backupComponents->StartSnapshotSet!\n");
        if (result == E_INVALIDARG)log.printError("One of the parameter values is not valid.");
        if (result == VSS_E_SNAPSHOT_SET_IN_PROGRESS)log.printError("The creation of a shadow copy is in progress, and only one shadow copy creation operation can be in progress at one time. Either wait to try again or return with a failure error code.");
        if (result == E_OUTOFMEMORY)log.printError("The caller is out of memory or other system resources.");
        if (result == VSS_E_BAD_STATE)log.printError("The backup components object is not initialized, this method has been called during a restore operation, or this method has not been called within the correct sequence.");
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: StartSnapshotSet() was successfully executed.\n");
    if (!SUCCEEDED(result = backupComponents->AddToSnapshotSet(vol, GUID_NULL, &snapshotId)))
    {

        log.printError("Error in AddToSnapshotSet()\n");
        if (result == E_ACCESSDENIED)
            log.printError("The caller does not have sufficient backup privileges or is not an administrator.");
        if (result == E_INVALIDARG)
            log.printError("One of the parameter values is not valid");
        if (result == VSS_E_BAD_STATE)
            log.printError("The backup components object is not initialized, this method has been called during a restore operation, or this method has not been called within the correct sequence.");
        if (result == VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED)
            log.printError("The maximum number of volumes or remote file shares have been added to the shadow copy set. The specified volume or remote file share was not added to the shadow copy set.");
        if (result == VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED)
            log.printError("The volume or remote file share has been added to the maximum number of shadow copy sets. The specified volume or remote file share was not added to the shadow copy set.");
        if (result == E_OUTOFMEMORY)
            log.printError("The caller is out of memory or other system resources.");
        if (result == 0x8004232CL)
            log.printError("The specified volume is nested too deeply to participate in the VSS operation. Possible reasons for this error include the following:\n"
                           "Trying to create a shadow copy of a volume that resides on a VHD that is contained in another VHD.\n"
                           "Trying to create a shadow copy of a VHD volume when the volume that contains the VHD is also in the same shadow copy set.");
        if (result == VSS_E_OBJECT_NOT_FOUND)
            log.printError("pwszVolumeName does not correspond to an existing volume or remote file share");
        if (result == VSS_E_PROVIDER_NOT_REGISTERED)
            log.printError("ProviderId does not correspond to a registered provider.");
        if (result == VSS_E_PROVIDER_VETO)
            log.printError("Expected provider error. The provider logged the error in the event log. For more information, see Event and Error Handling Under VSS.");
        if (result == VSS_E_SNAPSHOT_SET_IN_PROGRESS)
            log.printError("Another shadow copy creation is already in progress. Occurs when adding a CSV volume to a snapshot set from multiple nodes at the same time, or while adding a scale out share to the snapshot set from multiple SMB client nodes at the same time.");
        if (result == VSS_E_VOLUME_NOT_SUPPORTED)
            log.printError("The value of the ProviderId parameter is GUID_NULL and no VSS provider indicates that it supports the specified volume or remote file share.");
        if (result == VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER)
            log.printError("The volume or remote file share is not supported by the specified provider.");
        if (result == VSS_E_UNEXPECTED_PROVIDER_ERROR)
            log.printError("The provider returned an unexpected error code. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.");
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: AddToSnapshotSet() was successfully executed.\n");
    IVssAsync *async;
    if (!SUCCEEDED(result = backupComponents->DoSnapshotSet(&async)))
    {
        log.printError("Error in backupComponents->DoSnapshotSet!\n");
        if (result == E_ACCESSDENIED)
            log.printError("The caller does not have sufficient backup privileges or is not an administrator.");
        if (result == E_INVALIDARG)
            log.printError("ppAsyncdoes not point to a valid pointer; that is, it is NULL.");
        if (result == E_OUTOFMEMORY)
            log.printError("aThe caller is out of memory or other system resources.");
        if (result == VSS_E_BAD_STATE)
            log.printError("The backup components object has not been initialized or the prerequisite calls for a given shadow copy context have not been made prior to calling DoSnapshotSet.");
        if (result == VSS_E_INSUFFICIENT_STORAGE)
            log.printError("The system or provider has insufficient storage space. If possible delete any old or unnecessary persistent shadow copies and try again. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.");
        if (result == VSS_E_FLUSH_WRITES_TIMEOUT)
            log.printError("The system was unable to flush I/O writes. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times.");
        if (result == VSS_E_HOLD_WRITES_TIMEOUT)
            log.printError("The system was unable to hold I/O writes. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times.");
        if (result == VSS_E_NESTED_VOLUME_LIMIT)
            log.printError("The specified volume is nested too deeply to participate in the VSS operation.");
        if (result == VSS_E_PROVIDER_VETO)
            log.printError("The provider was unable to perform the request at this time. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times. "
                           "This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.");
        if (result == VSS_E_REBOOT_REQUIRED)
            log.printError("The provider encountered an error that requires the user to restart the computer.");
        if (result == VSS_E_TRANSACTION_FREEZE_TIMEOUT)
            log.printError("The system was unable to freeze the Distributed Transaction Coordinator (DTC) or the Kernel Transaction Manager (KTM).");
        if (result == VSS_E_TRANSACTION_THAW_TIMEOUT)
            log.printError("The system was unable to thaw the Distributed Transaction Coordinator (DTC) or the Kernel Transaction Manager (KTM).");
        if (result == VSS_E_UNEXPECTED)
            log.printError("Unexpected error. The error code is logged in the error log file. For more information, see Event and Error Handling Under VSS.");
        if (result == VSS_E_UNEXPECTED_PROVIDER_ERROR)
            log.printError("The provider returned an unexpected error code. This can be a transient problem. It is recommended to wait ten minutes and try again, up to three times. This error code is only returned via the QueryStatus method on the IVssAsync interface returned in the ppAsync parameter.");
        log.printError("\n",0);//*/
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: DoSnapshotSet() was successfully executed.\n");
    result = async->Wait();
    async->Release();

    if (!SUCCEEDED(result))
    {
        log.printError("Error wait for snapshot create! ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: async->Wait() was successfully executed.\n");
    VSS_SNAPSHOT_PROP prop;
    result = backupComponents->GetSnapshotProperties(snapshotId, &prop);
    if (!SUCCEEDED(result))
    {
        log.printError("Handle error in GetSnapshotProperties! ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 2)
    {
        char str[MAX_PATH*2] = {0};
        sprintf(str,"INFO: The shadow copy was successfully created for volume %ws \n"
                    "Devise: %ws \n"
                    "Original volume: %ws \n"
                    "Machine: %ws \n"
                    "Service Machine: %ws \n",vol,prop.m_pwszSnapshotDeviceObject,prop.m_pwszOriginalVolumeName,prop.m_pwszOriginatingMachine,prop.m_pwszServiceMachine);
        log.printError(str);
    }
    if (!rawMode)
    {
        std::wcout << prop.m_pwszSnapshotDeviceObject << "\n";
        backupComponents->Release();
        FreeLibrary(vssapidll);
        FreeLibrary(ole32dll);
        if (logLevel > 3) log.printError("DEBUG: All operations comleated, goodbye!\n");
        return 0;
    }
    if (logLevel > 3) log.printError("DEBUG: Prepare to raw mode.\n");
    std::ofstream rfile;
    DWORD nRead;
    unsigned long long rRead = 0;
    unsigned int buffSize = 4096;
    unsigned char buf[4096];
    unsigned long long DiskSize;
    unsigned long long FreeBytesAvailable;
    unsigned long long TotalNumberOfFreeBytes;
    if(!GetDiskFreeSpaceEx(vol,
                           (PULARGE_INTEGER)&FreeBytesAvailable,
                           (PULARGE_INTEGER)&DiskSize,
                           (PULARGE_INTEGER)&TotalNumberOfFreeBytes
                           ))
    {
        log.printError("ERROR: Can not get destination volume size. ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3)
    {
        char str[MAX_PATH*2] = {0};
        sprintf(str,"DEBUG: Source disk size detected us %llu bytes.\n",DiskSize);
        log.printError(str);
    }
    HANDLE hDisk = CreateFile(prop.m_pwszSnapshotDeviceObject,
                              GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, 0, NULL);
    if (!hDisk)
    {
        log.printError("ERROR: Can not open snapshot device object. ");
        char s[MAX_PATH] = {0};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
        log.printError("\n",0);
        return -1;
    }
    if (logLevel > 3) log.printError("DEBUG: The snapshot device object was successfully opened.\n");
    if (rawfile)
    {
        rfile.open(rawfile,std::ios_base::binary);
        if (!rfile.is_open())
        {
            log.printError("ERROR: Can not open output file: ");
            log.printError(rawfile,0);
            log.printError(" for raw output! ",0);
            log.printError(strerror(errno),0);
            log.printError("\n",0);
            return -1;
        }
        if (logLevel > 3) log.printError("DEBUG: The output file was successfully opened\n");

    }
    if (logLevel > 3) log.printError("DEBUG: Try to start transfer.\n");
    for (ULONGLONG i = DiskSize;i >= buffSize;i=i-buffSize)
    {
        ReadFile(hDisk, buf, buffSize, &nRead, NULL);
        if (!nRead)
        {
            log.printError("ERROR: Can not read the shadow copy. ");
            char s[MAX_PATH] = {0};
            if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),1033,s,MAX_PATH-1,NULL))log.printError(s,0);
            log.printError("\n",0);
        }
        rRead = rRead+nRead;
        if (rawfile)
        {
            rfile.write((char*)&buf,buffSize);
            if (rfile.bad())
            {
                if(networkMode)
                {
                    //отрефакорить это дерьмо;
                    rfile.close();
                    rfile.open(rawfile,std::ios_base::binary);
                    {

                    }

                }
                log.printError("ERROR: Can not write to ");
                log.printError(rawfile,0);
                log.printError(" file. ",0);
                log.printError(strerror(errno),0);
                log.printError("\n",0);
                return -1;
            }
            if (i==buffSize) rfile.close();
        }else std::cout.write((char*)&buf,buffSize);
    }
    //сделать это говно функцией, получать хвост остатком от деления
    if (rRead < DiskSize)
    {
        if (logLevel > 3) log.printError("DEBUG: Tail smaller 4096 bytes detected. Writing it in byte mode.\n");
        ReadFile(hDisk, buf, (DWORD)(DiskSize - rRead), &nRead, NULL);
        rRead = rRead+nRead;
        for (unsigned int i=0;i < nRead;i++)
        {
            if (rawfile)
            {
                rfile.write((char*)&buf[i],1);
                if (rfile.bad())
                {
                    log.printError("ERROR: Can not write to ");
                    log.printError(rawfile,0);
                    log.printError(" file. ");
                    log.printError(strerror(errno),0);
                    log.printError("\n",0);
                    return -1;
                }
            }else std::cout.write((char*)&buf[i],1);
        }
        if (logLevel > 3) log.printError("DEBUG: Transfer comleted, close files.\n");
        if (rawfile)
        {
            rfile << std::flush;
            rfile.close();
        }else std::cout << std::flush;
    }
    CloseHandle(hDisk);
    if (logLevel > 2)
    {
        char s[MAX_PATH] = {0};
        sprintf(s,"INFO: %llu bytes transfered\n",rRead);
        log.printError(s);
    }
    backupComponents->Release();
    FreeLibrary(vssapidll);
    FreeLibrary(ole32dll);
    if (logLevel > 3) log.printError("DEBUG: All operations comleated, goodbye!\n");
    return 0;
}
