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
    std::shared_ptr<char> fileName;
    DWORD filter;
    char rank;

    File(const DWORDLONG aParentIndex=0, const QString aFileName="", const DWORD aFilter=0, const char aRank=0)
    {
        parentIndex = aParentIndex;
        filter = aFilter;
        rank = aRank;

        QByteArray fileNameU8 = aFileName.toUtf8();
        char* tmp_char_ptr = new char[fileNameU8.size() + 1];
        strncpy(tmp_char_ptr, fileNameU8.data(), fileNameU8.size() + 1);
        fileName.reset(tmp_char_ptr);
    }

    QString getStrName() const
    {
        return QString::fromUtf8(QByteArray(fileName.get()));
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
    unsigned char m_state;     // 0:free 1:build 2:update 3:Serialization
    HANDLE      m_hVol;			// handle to volume
    WCHAR       m_drive;		// drive letter of volume
    DWORDLONG   m_driveFRN;     // drive FileReferenceNumber
    FileMap     m_FileMap;
    QMutex      m_FileMapMutex;
    bool        m_StopFind;
    USN         m_StartUSN;
    USN_JOURNAL_DATA m_ujd;

    void CleanUp();

    void SerializationWrite();
    void SerializationRead();

    HANDLE Open(WCHAR cDriveLetter, DWORD dwAccess);
    bool Query(PUSN_JOURNAL_DATA pUsnJournalData);

    DWORD MakeFilter(QString* str);
    char* SimplifyString(QString* str);

    bool AddFile(DWORDLONG Index, wstring fileName, DWORDLONG ParentIndex);
    char MatchStr(const QString &contain, const QString &query);

    bool GetPath(DWORDLONG Index, QString *sz);
    char GetFileRank(QString *filename);
};

#endif // VOLUME_H
