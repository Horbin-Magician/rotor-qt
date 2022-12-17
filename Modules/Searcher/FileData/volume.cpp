#include "volume.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <QDebug>
#include <QElapsedTimer>

#include "Models/setting_model.h"

// Constructor
Volume::Volume(WCHAR drive)
{
    m_StopFind = false;
    // Initialize member variables
    m_drive = drive; // drive letter of volume
    m_driveFRN = 0x5000000000005; // drive FileReferenceNumber
    m_StartUSN = 0x0;
    // Open a handle to the volume
    m_hVol = Open(m_drive, GENERIC_READ); // UPDATA:20220910 del flag "GENERIC_WRITE"
    if (INVALID_HANDLE_VALUE == m_hVol) CleanUp();
}

// Destructor
Volume::~Volume()
{
    CleanUp();
}

// Cleanup function to free resources
void Volume::CleanUp()
{
    // Cleanup the memory and handles we were using
    if (m_hVol != INVALID_HANDLE_VALUE)
        CloseHandle(m_hVol);
    ReleaseIndex();
}

// Clears the database
BOOL Volume::ReleaseIndex()
{
    m_FileMap.clear();
    return TRUE;
}

// This is a helper function that opens a handle to the volume specified by the cDriveLetter parameter.
HANDLE Volume::Open(TCHAR cDriveLetter, DWORD dwAccess){
    TCHAR szVolumePath[_MAX_PATH];
    wsprintf(szVolumePath, TEXT("\\\\.\\%c:"), cDriveLetter);
    HANDLE hCJ = CreateFile(szVolumePath, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    return(hCJ);
}

// Return statistics about the journal on the current volume
BOOL Volume::Query(PUSN_JOURNAL_DATA pUsnJournalData){
    DWORD cb;
    BOOL fOk = DeviceIoControl(m_hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, pUsnJournalData, sizeof(*pUsnJournalData), &cb, NULL);
    return(fOk);
}

// Enumerate the MFT for all entries. Store the file reference numbers of any directories in the database.
void Volume::BuildIndex(){
    QElapsedTimer timedebuge;
    timedebuge.start();

    ReleaseIndex();

    Query(&m_ujd);
    m_StartUSN = m_ujd.NextUsn;

    // add the root directory
    WCHAR szRoot[_MAX_PATH];
    wsprintf(szRoot, TEXT("%c:"), m_drive);
    AddFile(m_driveFRN, (wstring)szRoot, 0);

    MFT_ENUM_DATA med;
    med.StartFileReferenceNumber = 0;
    med.LowUsn = 0;
    med.HighUsn = m_ujd.NextUsn;

    BYTE pData[sizeof(DWORDLONG) * 0x10000];
    DWORD cb;

    while (DeviceIoControl(m_hVol, FSCTL_ENUM_USN_DATA, &med, sizeof(med), pData, sizeof(pData), &cb, NULL)){
        PUSN_RECORD pRecord = (PUSN_RECORD) &pData[sizeof(USN)];
        while ((PBYTE) pRecord < (pData + cb)){
            wstring sz((LPCWSTR) ((PBYTE) pRecord + pRecord->FileNameOffset), pRecord->FileNameLength / sizeof(WCHAR));
            AddFile(pRecord->FileReferenceNumber, sz, pRecord->ParentFileReferenceNumber);
            pRecord = (PUSN_RECORD) ((PBYTE) pRecord + pRecord->RecordLength);
        }
        med.StartFileReferenceNumber = * (USN *) pData;
    }
    ReduceIndex();
    qDebug()<<(char)m_drive<<"构建耗时："<<timedebuge.elapsed()<<"ms";
}

// Delete ignored and useless index
void Volume::ReduceIndex(){
    SettingModel& settingModel = SettingModel::getInstance();
    QStringList ignoredPathList = settingModel.getIgnoredPath();

    for(FileMap::iterator it = m_FileMap.begin();it != m_FileMap.end();){
        wstring path;
        wstring filename = it->filename;
        // del ignored path
        bool ifFound = false;
        for(int i=0; i < ignoredPathList.length(); i++){
            if(ifFound == false){
                wstring strQuery = ignoredPathList[i].toStdWString();
                QStringList splitList = ignoredPathList[i].split('\\');
                wstring nameQuery = splitList[splitList.length()-1].toStdWString();
                if(filename == nameQuery && GetPath(it->parentIndex, &path)){
                    if(path + filename == strQuery) ifFound = true;
                }
            }
        }
        if(ifFound)it = m_FileMap.erase(it);
        else ++it;
    }

    for(FileMap::iterator it = m_FileMap.begin();it != m_FileMap.end();){
        // del file with ignored path or useless
        wstring path;
        if(GetPath(it->parentIndex, &path) == FALSE){
            it = m_FileMap.erase(it);
            continue;
        }
        it++;
    }
}

void Volume::UpdateIndex(){
    WCHAR szRoot[_MAX_PATH];
    wsprintf(szRoot, TEXT("%c:"), m_drive);

    BYTE pData[sizeof(DWORDLONG) * 0x10000];
    DWORD cb;
    DWORD reason_mask = USN_REASON_FILE_CREATE | USN_REASON_FILE_DELETE | USN_REASON_RENAME_NEW_NAME;
    READ_USN_JOURNAL_DATA rujd = {m_StartUSN, reason_mask, 0, 0, 0, m_ujd.UsnJournalID};

    while (DeviceIoControl(m_hVol, FSCTL_READ_USN_JOURNAL, &rujd, sizeof(rujd), pData, sizeof(pData), &cb, NULL)){
        if(cb == 8) break;
        PUSN_RECORD pRecord = (PUSN_RECORD) &pData[sizeof(USN)];
        while ((PBYTE) pRecord < (pData + cb)){
            wstring sz((LPCWSTR) ((PBYTE) pRecord + pRecord->FileNameOffset), pRecord->FileNameLength / sizeof(WCHAR));
            if ((pRecord->Reason & USN_REASON_FILE_CREATE) == USN_REASON_FILE_CREATE){
                AddFile(pRecord->FileReferenceNumber, sz, pRecord->ParentFileReferenceNumber);
            }
            else if ((pRecord->Reason & USN_REASON_FILE_DELETE) == USN_REASON_FILE_DELETE){
                m_FileMap.remove(pRecord->FileReferenceNumber);
            }
            else if ((pRecord->Reason & USN_REASON_RENAME_NEW_NAME) == USN_REASON_RENAME_NEW_NAME){
                AddFile(pRecord->FileReferenceNumber, sz, pRecord->ParentFileReferenceNumber);
            }
            pRecord = (PUSN_RECORD) ((PBYTE) pRecord + pRecord->RecordLength);
        }
        rujd.StartUsn = *(USN *)&pData;
    }
}

void Volume::StopFind(){
    m_StopFind = true;
}

// Adds a file to the database
BOOL Volume::AddFile(DWORDLONG index, wstring filename, DWORDLONG parentIndex){
    char rank = GetFileRank(&filename);
    File insertFile(parentIndex, filename, rank);
    m_FileMap[index] = insertFile;
    return(TRUE);
}

// searching
vector<SearchResultFile>* Volume::Find(wstring strQuery){
    vector<SearchResultFile>* rgsrfResults = new vector<SearchResultFile>();
    if(strQuery.length() == 0) return rgsrfResults; //No query, just ignore this call

    //Create lower query string for case-insensitive search
    for(unsigned int j = 0; j != strQuery.length(); ++j)
        strQuery[j] = tolower(strQuery[j]);

    for(QMap<DWORDLONG, File>::iterator it = m_FileMap.begin(); it != m_FileMap.end(); ++it){
        if(m_StopFind){
            m_StopFind = false;
            delete rgsrfResults;
            return nullptr;
        }

        wstring szLower = it->filename;
        for(unsigned int j = 0; j != szLower.length(); ++j)
            szLower[j] = tolower(szLower[j]);
        if(szLower.find(strQuery) != -1){
            SearchResultFile srf;
            srf.path.reserve(MAX_PATH);
            if(GetPath(it->parentIndex, &srf.path)){
                srf.filename = it->filename;
                srf.rank = it->rank;
                rgsrfResults->insert(rgsrfResults->end(), srf);
            }
        }
    }
    return rgsrfResults;
}

// Constructs a path for a directory
BOOL Volume::GetPath(DWORDLONG index, wstring *sz)
{
    *sz = TEXT("");
    while (index != 0){
        if(m_FileMap.contains(index) == FALSE) return FALSE;
        File file = m_FileMap[index];
        *sz = file.filename + TEXT("\\") + *sz;
        index = file.parentIndex;
    };
    return(TRUE);
}

// return rank by filename
char Volume::GetFileRank(wstring *filename)
{
    char rank = -5;
    if(filename->find(L".exe", filename->size() - 4) != -1)rank += 10;
    else if(filename->find(L".lnk", filename->size() - 4) != -1) rank += 15;
    return rank;
}
