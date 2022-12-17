#ifndef VOLUME_H
#define VOLUME_H

#include <QMap>
#include <string>
#include <Windows.h>
#include <WinIoCtl.h>

using namespace std;

struct File
{
    wstring filename;
    DWORDLONG parentIndex;
    char rank;

    File(const DWORDLONG aParentIndex=0, const wstring aFilename=TEXT(""), const char aRank=0)
    {
        parentIndex = aParentIndex;
        rank = aRank;
        filename = aFilename;
    }
};

struct SearchResultFile
{
    wstring filename;
    wstring path;
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

class Volume {
public:
    Volume(WCHAR cDrive);
    ~Volume();
    vector<SearchResultFile>* Find(wstring strQuery);
    void BuildIndex();
    void UpdateIndex();
    void StopFind();
private:
    HANDLE      m_hVol;			// handle to volume
    WCHAR       m_drive;		// drive letter of volume
    DWORDLONG   m_driveFRN;     // drive FileReferenceNumber
    FileMap     m_FileMap;
    bool        m_StopFind;
    USN         m_StartUSN;
    USN_JOURNAL_DATA m_ujd;

    void CleanUp();
    BOOL ReleaseIndex();
    void ReduceIndex();

    HANDLE Open(WCHAR cDriveLetter, DWORD dwAccess);
    BOOL Query(PUSN_JOURNAL_DATA pUsnJournalData);

    BOOL AddFile(DWORDLONG Index, wstring filename, DWORDLONG ParentIndex);
    BOOL GetPath(DWORDLONG Index, wstring *sz);

    char GetFileRank(wstring *filename);
};

#endif // VOLUME_H
