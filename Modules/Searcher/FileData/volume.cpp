#include "volume.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>

#include "Models/setting_model.h"

// Constructor
Volume::Volume(WCHAR drive)
{
    m_state = 0;
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

// Clears the database
void Volume::ReleaseIndex(bool ifLock)
{
    if(ifLock) m_FileMapMutex.lock();
    m_FileMap.clear();
    if(ifLock) m_FileMapMutex.unlock();
}

// Cleanup function to free resources
void Volume::CleanUp()
{
    // Cleanup the memory and handles we were using
    if (m_hVol != INVALID_HANDLE_VALUE)
        CloseHandle(m_hVol);
    ReleaseIndex();
}

// This is a helper function that opens a handle to the volume specified by the cDriveLetter parameter.
HANDLE Volume::Open(TCHAR cDriveLetter, DWORD dwAccess){
    TCHAR szVolumePath[_MAX_PATH];
    wsprintf(szVolumePath, TEXT("\\\\.\\%c:"), cDriveLetter);
    HANDLE hCJ = CreateFile(szVolumePath, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    return(hCJ);
}

// Return statistics about the journal on the current volume
bool Volume::Query(PUSN_JOURNAL_DATA pUsnJournalData){
    DWORD cb;
    BOOL fOk = DeviceIoControl(m_hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, pUsnJournalData, sizeof(*pUsnJournalData), &cb, NULL);
    return(fOk);
}

// Enumerate the MFT for all entries. Store the file reference numbers of any directories in the database.
void Volume::BuildIndex(){
    QElapsedTimer timer; //定义对象
    timer.start();  //开始计时

    m_FileMapMutex.lock();

    ReleaseIndex(false);

    Query(&m_ujd);
    m_StartUSN = m_ujd.NextUsn;

    // add the root directory
    WCHAR szRoot[_MAX_PATH];
    wsprintf(szRoot, TEXT("%c:"), m_drive);
    AddFile(m_driveFRN, szRoot, 0);

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

    SerializationWrite();
    m_FileMapMutex.unlock();

    qDebug() << m_drive  << "[计时信息] BuildIndex用时：" << timer.elapsed() << "milliseconds";
}

void Volume::UpdateIndex(){
    if(m_FileMap.isEmpty()) SerializationRead();

    WCHAR szRoot[_MAX_PATH];
    wsprintf(szRoot, TEXT("%c:"), m_drive);

    BYTE pData[sizeof(DWORDLONG) * 0x10000];
    DWORD cb;
    DWORD reason_mask = USN_REASON_FILE_CREATE | USN_REASON_FILE_DELETE | USN_REASON_RENAME_NEW_NAME;
    READ_USN_JOURNAL_DATA rujd = {m_StartUSN, reason_mask, 0, 0, 0, m_ujd.UsnJournalID};

    qDebug()<<m_StartUSN;

    m_FileMapMutex.lock();
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
    m_FileMapMutex.unlock();

    m_StartUSN = rujd.StartUsn;
}

void Volume::StopFind(){
    m_StopFind = true;
}

void Volume::SerializationWrite()
{
    if(m_FileMap.isEmpty()) return;

    QString appPath = QApplication::applicationDirPath(); // get programe path
    QFile file(appPath + "/userdata/" + m_drive + ".fd");
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QDataStream out(&file);

    out<<m_StartUSN;

    QMapIterator<DWORDLONG, File> i(m_FileMap);
    while (i.hasNext()){
        i.next();
        File filedata = i.value();
        out<<i.key()<<filedata.parentIndex<<filedata.fileName.get()<<(quint32)filedata.filter<<filedata.rank;
    }

    this->ReleaseIndex(false);
    file.close();
}

void Volume::SerializationRead()
{
    QString appPath = QApplication::applicationDirPath(); // get programe path
    QFile file(appPath + "/userdata/" + m_drive + ".fd");
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    DWORDLONG index;
    DWORDLONG parentIndex;
    QByteArray fileName;
    quint32 filter;
    char rank;

    in>>m_StartUSN;

    m_FileMapMutex.lock();
    while(in.atEnd() == false){
        in>>index>>parentIndex>>fileName>>filter>>rank;
        File insertFile(parentIndex, fileName, filter, rank);
        m_FileMap[index] = insertFile;
    }
    m_FileMapMutex.unlock();
    file.close();
}

// Adds a file to the database
bool Volume::AddFile(DWORDLONG index, wstring fileName, DWORDLONG parentIndex){
    QString qFileName = QString::fromStdWString(fileName);
    DWORD filter = MakeFilter(&qFileName);
    char rank = GetFileRank(&qFileName);
    File insertFile(parentIndex, qFileName, filter, rank);
    m_FileMap[index] = insertFile;
    return(TRUE);
}

char Volume::MatchStr(const QString &contain, const QString &query_lower)
{
    int i = 0;
    foreach (QChar c, contain) {
        if(query_lower[i] == c) ++i;
        if(i >= query_lower.length()){
            int rank = 20 - (contain.length() - query_lower.length());
            return rank < 0 ? 0 : rank;
        }
    }
    return -1;
}

// searching
vector<SearchResultFile>* Volume::Find(QString strQuery){
    if(strQuery.length() == 0) return nullptr;
    if(m_FileMap.isEmpty()) SerializationRead();

    QString strQuery_lower = strQuery.toLower();

    vector<SearchResultFile>* rgsrfResults = new vector<SearchResultFile>();

    DWORD queryFilter = MakeFilter(&strQuery_lower); //Calculate Filter value which are compared with the cached ones to skip many of them

    m_FileMapMutex.tryLock(1);

    for(QMap<DWORDLONG, File>::iterator it = m_FileMap.begin(); it != m_FileMap.end(); ++it){
        if(m_StopFind){
            m_StopFind = false;
            delete rgsrfResults;
            return nullptr;
        }

        if((it->filter & queryFilter) == queryFilter){
            QString sz = it->getStrName();
            char rank = MatchStr(sz, strQuery_lower);
            if(rank > 0){
                SearchResultFile srf;
                srf.path.reserve(MAX_PATH);
                if(GetPath(it->parentIndex, &srf.path)){
                    srf.filename = sz;
                    srf.rank = it->rank + rank;
                    rgsrfResults->insert(rgsrfResults->end(), srf);
                }
            }
        }
    }
    m_FileMapMutex.unlock();
    return rgsrfResults;
}

// Constructs a path for a directory
bool Volume::GetPath(DWORDLONG index, QString *sz)
{
    *sz = "";
    while (index != 0){
        if(m_FileMap.contains(index) == false) return false;
        File file = m_FileMap[index];
        *sz = file.getStrName() + "\\" + *sz;
        index = file.parentIndex;
    };
    return TRUE;
}

// return rank by filename
char Volume::GetFileRank(QString *fileName)
{
    char rank = 0;
    if(fileName->endsWith(L".exe", Qt::CaseInsensitive))rank += 10;
    else if(fileName->endsWith(L".lnk", Qt::CaseInsensitive)) rank += 30;
    return rank;
}

// Calculates a 32bit value that is used to filter out many files before comparing their filenames
DWORD Volume::MakeFilter(QString* str)
{
    /*
    Creates an address that is used to filter out strings that don't contain the queried characters
    Explanation of the meaning of the single bits:
    0-25 a-z
    26 0-9
    27 other ASCII
    28 not in ASCII
    */
    uint len = str->length();
    if(len <= 0) return 0;
    uint32_t Address = 0;

    QString szlower = str->toLower();

    char c;

    for(uint i = 0; i != len; ++i)
    {
        c = szlower[i].toLatin1();
        if(c > 96 && c < 123) Address |= (uint32_t)1 << ((uint32_t)c - (uint32_t)97); //a-z
        else if(c >= L'0' && c <= '9') Address |= (uint32_t)1 << 26; //0-9
        else if(c == 0) Address |= (uint32_t)1 << 28; // not in ASCII
        else Address |= (uint32_t)1 << 27; // other ASCII
    }
    return Address;
}
