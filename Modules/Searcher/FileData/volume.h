#ifndef VOLUME_H
#define VOLUME_H

#include <QMap>
#include <QFile>
#include <QMutex>
#include <QApplication>
#include <string>
#include <Windows.h>
#include <WinIoCtl.h>

using namespace std;

struct File
{
    DWORDLONG parentIndex;
    QString filename;
    DWORD filter;
    short rank;

    File(const DWORDLONG aParentIndex=0, const QString aFilename="", const DWORD aFilter=0, const short aRank=0)
    {
        parentIndex = aParentIndex;
        filter = aFilter;
        rank = aRank;
        filename = aFilename;
    }
};

struct SearchResultFile
{
    QString filename;
    QString path;
    char rank;

    bool operator<(const SearchResultFile& i){
        return rank > i.rank;
    }
};

struct SearchResult
{
    wstring Query;
    vector<SearchResultFile> Results;
    int maxResults;
    SearchResult()
    {
        Query = wstring();
        Results = vector<SearchResultFile>();
        maxResults = -1;
    }
};

typedef QMap<DWORDLONG, File> FileMap;

class Volume{
public:
    Volume(WCHAR cDrive);
    ~Volume();
    vector<SearchResultFile>* Find(QString strQuery);
    void BuildIndex();
    void UpdateIndex();
    void ReleaseIndex(bool ifLock = true);
    void StopFind();
private:
    unsigned short m_state;     // 0:free 1:build 2:update 3:Serialization
    HANDLE      m_hVol;			// handle to volume
    WCHAR       m_drive;		// drive letter of volume
    DWORDLONG   m_driveFRN;     // drive FileReferenceNumber
    FileMap     m_FileMap;
    QMutex      m_FileMapMutex;
    bool        m_StopFind;
    USN         m_StartUSN;
    USN_JOURNAL_DATA m_ujd;

    void CleanUp();
    void ReduceIndex();

    void SerializationWrite();
    void SerializationRead();

    HANDLE Open(WCHAR cDriveLetter, DWORD dwAccess);
    bool Query(PUSN_JOURNAL_DATA pUsnJournalData);

    DWORD MakeFilter(QString* str);
    bool AddFile(DWORDLONG Index, QString filename, DWORDLONG ParentIndex);
    bool GetPath(DWORDLONG Index, QString *sz);

    short GetFileRank(QString *filename);
};

#endif // VOLUME_H
