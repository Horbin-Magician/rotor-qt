#include "volume.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

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

    USN_JOURNAL_DATA ujd;
    Query(&ujd);

    // add the root directory
    WCHAR szRoot[_MAX_PATH];
    wsprintf(szRoot, TEXT("%c:"), m_drive);
    AddFile(m_driveFRN, (wstring)szRoot, 0);

    MFT_ENUM_DATA med;
    med.StartFileReferenceNumber = 0;
    med.LowUsn = 0;
    med.HighUsn = ujd.NextUsn;

    BYTE pData[sizeof(DWORDLONG) * 0x10000];
    DWORD cb;

    while (DeviceIoControl(m_hVol, FSCTL_ENUM_USN_DATA, &med, sizeof(med), pData, sizeof(pData), &cb, NULL) != FALSE){
        PUSN_RECORD pRecord = (PUSN_RECORD) &pData[sizeof(USN)];
        while ((PBYTE) pRecord < (pData + cb)){
            wstring sz((LPCWSTR) ((PBYTE) pRecord + pRecord->FileNameOffset), pRecord->FileNameLength / sizeof(WCHAR));

            AddFile(pRecord->FileReferenceNumber, sz, pRecord->ParentFileReferenceNumber);
            pRecord = (PUSN_RECORD) ((PBYTE) pRecord + pRecord->RecordLength);
        }
        med.StartFileReferenceNumber = * (USN *) pData;
    }
    qDebug()<<(char)m_drive<<"构建耗时："<<timedebuge.elapsed()<<"ms";
    ReduceIndex();

    QFuture<void> future = QtConcurrent::run([=](){UpdateIndex();});
}

void Volume::UpdateIndex(){
    WCHAR szRoot[_MAX_PATH];
    wsprintf(szRoot, TEXT("%c:"), m_drive);

    USN_JOURNAL_DATA ujd;
    Query(&ujd);

    BYTE pData[sizeof(DWORDLONG) * 0x10000];
    DWORD cb;
    READ_USN_JOURNAL_DATA rujd = {m_StartUSN, 4294967295, 0, 0, 0, ujd.UsnJournalID};

    while (DeviceIoControl(m_hVol, FSCTL_READ_USN_JOURNAL, &rujd, sizeof(rujd), pData, sizeof(pData), &cb, NULL)){
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
                DWORDLONG queryFilter = MakeFilter(&nameQuery);
                if(it->filter == queryFilter && filename == nameQuery && GetPath(it->parentIndex, &path)){
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

// Adds a file to the database
BOOL Volume::AddFile(DWORDLONG index, wstring filename, DWORDLONG parentIndex){
    DWORDLONG filter = MakeFilter(&filename);
    char rank = GetFileRank(&filename);
    File insertFile(parentIndex, filename, filter, rank);
    m_FileMap[index] = insertFile;
    return(TRUE);
}

// Calculates a 64bit value that is used to filter out many files before comparing their filenames
DWORDLONG Volume::MakeFilter(wstring *szName)
{
    /*
    Creates an address that is used to filter out strings that don't contain the queried characters
    Explanation of the meaning of the single bits:
    0-25 a-z
    26-35 0-9
    36 .
    37 space
    38 !#$&'()+,-~_
    39 2 same characters
    40 3 same characters
    The fields below indicate the presence of 2-character sequences. Based off http://en.wikipedia.org/wiki/Letter_frequency
    41 TH
    42 HE
    43 AN
    44 RE
    45 ER
    46 IN
    47 ON
    48 AT
    49 ND
    50 ST
    51 ES
    52 EN
    53 OF
    54 TE
    55 ED
    56 OR
    57 TI
    58 HI
    59 AS
    60 TO
    61-63 length (max. 8 characters. Queries are usually shorter than this)
    */
    if(szName->length() <= 0) return 0;
    DWORDLONG Address = 0;
    WCHAR c;
    wstring szlower(*szName);
    for(unsigned int j = 0; j != szlower.length(); ++j) szlower[j] = tolower(szlower[j]);

    int counts[26] = {0}; //This array is used to check if characters occur two or three times in the string
    wstring::size_type l = szlower.length();
    for(unsigned int i = 0; i != l; ++i)
    {
        c = szlower[i];
        if(c > 96 && c < 123) //a-z
        {
            Address |= (uint64_t)1 << (DWORDLONG)((DWORDLONG)c - (uint64_t)97);
            counts[c-97]++;
            if(i < l - 1)
            {
                if(c == L't' && szlower[i+1] == L'h')  Address |= (uint64_t)1 << 41; //th
                else if(c == L'h' && szlower[i+1] == L'e') Address |= (uint64_t)1 << 42; //he
                else if(c == L'a' && szlower[i+1] == L'n') Address |= (uint64_t)1 << 43; //an
                else if(c == L'r' && szlower[i+1] == L'e') Address |= (uint64_t)1 << 44; //re
                else if(c == L'e' && szlower[i+1] == L'r') Address |= (uint64_t)1 << 45; //er
                else if(c == L'i' && szlower[i+1] == L'n') Address |= (uint64_t)1 << 46; //in
                else if(c == L'o' && szlower[i+1] == L'n') Address |= (uint64_t)1 << 47; //on
                else if(c == L'a' && szlower[i+1] == L't') Address |= (uint64_t)1 << 48; //at
                else if(c == L'n' && szlower[i+1] == L'd') Address |= (uint64_t)1 << 49; //nd
                else if(c == L's' && szlower[i+1] == L't') Address |= (uint64_t)1 << 50; //st
                else if(c == L'e' && szlower[i+1] == L's') Address |= (uint64_t)1 << 51; //es
                else if(c == L'e' && szlower[i+1] == L'n') Address |= (uint64_t)1 << 52; //en
                else if(c == L'o' && szlower[i+1] == L'f') Address |= (uint64_t)1 << 53; //of
                else if(c == L't' && szlower[i+1] == L'e') Address |= (uint64_t)1 << 54; //te
                else if(c == L'e' && szlower[i+1] == L'd') Address |= (uint64_t)1 << 55; //ed
                else if(c == L'o' && szlower[i+1] == L'r') Address |= (uint64_t)1 << 56; //or
                else if(c == L't' && szlower[i+1] == L'i') Address |= (uint64_t)1 << 57; //ti
                else if(c == L'h' && szlower[i+1] == L'i') Address |= (uint64_t)1 << 58; //hi
                else if(c == L'a' && szlower[i+1] == L's') Address |= (uint64_t)1 << 59; //as
                else if(c == L't' && szlower[i+1] == L'o') Address |= (uint64_t)1 << 60; //to
            }
        }
        else if(c >= L'0' && c <= '9') Address |= (uint64_t)1 << (c - L'0' +(uint64_t) 26); //0-9
        else if(c == L'.') Address |= (uint64_t)1 << 36; //.
        else if(c == L' ') Address |= (uint64_t)1 << 37; // space
        else if(c == L'!' || c == L'#' || c == L'$' || c == L'&' || c == L'\'' || c == L'(' || c == L')' || c == L'+' || c == L',' || c == L'-' || c == L'~' || c == L'_')
            Address |= (uint64_t)1 << 38; // !#$&'()+,-~_
    }
    for(unsigned int i = 0; i < 26; ++i)
    {
        if(counts[i] >= 2) Address |= (uint64_t)1 << 39;
        if(counts[i] >= 3) Address |= (uint64_t)1 << 40;
    }
    DWORDLONG length = (szlower.length() > 7 ? (uint64_t)7 : (DWORDLONG)szlower.length()) & (uint64_t)0x00000007; //3 bits for length -> 8 max
    Address |= length << (uint64_t)61;
    return Address;
}

// searching
int Volume::Find(wstring strQuery, vector<SearchResultFile> *rgsrfResults, int maxResults){
    int nResults = 0; //Number of results in this search.
    if(strQuery.length() == 0) return nResults; //No query, just ignore this call

    //Create lower query string for case-insensitive search
    for(unsigned int j = 0; j != strQuery.length(); ++j)
        strQuery[j] = tolower(strQuery[j]);

    //Calculate Filter value and length of the current query which are compared with the cached ones to skip many of them
    DWORDLONG queryFilter = MakeFilter(&strQuery);
    DWORDLONG queryLength = (queryFilter & (uint64_t)0xE000000000000000) >> (uint64_t)61; //Bits 61-63 for storing lengths up to 8
    queryFilter = queryFilter & (uint64_t)0x1FFFFFFFFFFFFFFF; //All but the last 3 bits

    for(QMap<DWORDLONG, File>::iterator it = m_FileMap.begin(); it != m_FileMap.end(); ++it){
        if(m_StopFind){
            m_StopFind = false;
            return -1;
        }
        DWORDLONG length = (it->filter & (uint64_t)0xE000000000000000) >> (uint64_t)61; //Bits 61-63 for storing lengths up to 8
        DWORDLONG filter = it->filter & (uint64_t)0x1FFFFFFFFFFFFFFF; //All but the last 3 bits

        if((filter & queryFilter) == queryFilter && queryLength <= length){
            wstring szLower = it->filename;
            for(unsigned int j = 0; j != szLower.length(); ++j)
                szLower[j] = tolower(szLower[j]);
            if(szLower.find(strQuery) != -1){
                nResults++;
                if(maxResults != -1 && nResults > maxResults)break;
                SearchResultFile srf;
                srf.path.reserve(MAX_PATH);
                if(GetPath(it->parentIndex, &srf.path)){
                    srf.filename = it->filename;
                    srf.rank = it->rank;
                    rgsrfResults->insert(rgsrfResults->end(), srf);
                }
            }
        }
    }
    return nResults;
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
