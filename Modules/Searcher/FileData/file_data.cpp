#include <windows.h>
#include <winioctl.h>
#include <QDebug>
#include <QThreadPool>

#include "file_data.h"


InitVolumeWork::InitVolumeWork(const char vol)
{
    setAutoDelete(true);
    m_vol = vol;
}

void InitVolumeWork::run()
{
    Volume *volume = new Volume(m_vol);
    volume->BuildIndex();
    emit finished(volume);
}

FindWork::FindWork(Volume *volume, QString filename)
{
    setAutoDelete(true);
    m_volume = volume;
    m_filename = filename;
}

void FindWork::run(){
    vector<SearchResultFile>* result = new vector<SearchResultFile>();
    m_volume->Find(m_filename.toStdWString(), result);
    emit finished(m_filename, result);
}

void FindWork::stop(){
    m_volume->StopFind();
}

FileData::FileData()
{
    state = 0;
}

FileData::~FileData()
{
    for(Volume* volume : m_volumes) delete volume;
}

bool FileData::initVolumes()
{
    state = 1;
    m_waitingInit = initValidVols();
    for (int i=0; i<m_vols.size(); ++i)
    {
        InitVolumeWork* work = new InitVolumeWork(m_vols.at(i));
        connect(work, &InitVolumeWork::finished, this, &FileData::onInitVolumeWorkFinished);
        QThreadPool::globalInstance()->start(work);
    }
    return true;
}

void FileData::findFile(QString filename)
{
    emit stopFind();
    if(state != 2 || m_findingName == filename) return;

    m_findingName = filename;
    if(filename.isEmpty()) return;

    m_findingResult.clear();
    m_waitingFinder = m_volumes.length();

    for(Volume* volume: m_volumes){
        FindWork* work = new FindWork(volume, m_findingName);
        connect(work, &FindWork::finished, this, &FileData::onFindWorkFinished);
        connect(this, &FileData::stopFind, work, &FindWork::stop);
        QThreadPool::globalInstance()->start(work);
    }
}

void FileData::onInitVolumeWorkFinished(Volume *volume)
{
    m_volumes.append(volume);
    m_waitingInit--;
    if(m_waitingInit == 0) state = 2;
}

void FileData::onFindWorkFinished(QString filename, vector<SearchResultFile>* result)
{
    if( filename != this->m_findingName) return;
    m_findingResult.insert(m_findingResult.end(), result->begin(), result->end());
    if(--m_waitingFinder == 0){
        sort(m_findingResult.begin(), m_findingResult.end());
        emit updateSearchResult(filename, m_findingResult);
    }
    delete result;
}

unsigned short FileData::initValidVols(){
    m_vols.empty();
    for (int i=0; i<26; ++i ){
        char cvol = i +'A';
        if ( isNTFS(cvol) ) m_vols.append(cvol);
    }
    return m_vols.length();
}

bool FileData::isNTFS(char vol)
{
    char lpRootPathName[] = ("t:\\");
    lpRootPathName[0] = vol;
    char lpVolumeNameBuffer[MAX_PATH];
    DWORD lpVolumeSerialNumber;
    DWORD lpMaximumComponentLength;
    DWORD lpFileSystemFlags;
    char lpFileSystemNameBuffer[MAX_PATH];

    if ( GetVolumeInformationA(lpRootPathName,lpVolumeNameBuffer, MAX_PATH,
            &lpVolumeSerialNumber, &lpMaximumComponentLength, &lpFileSystemFlags,
            lpFileSystemNameBuffer, MAX_PATH)) {
        return !strcmp(lpFileSystemNameBuffer, "NTFS");
    }
    return false;
}
