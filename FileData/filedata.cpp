#include "filedata.h"

#include <windows.h>
#include <winioctl.h>
#include <QDebug>
#include <QThreadPool>

InitVolumeWork::InitVolumeWork(const char vol)
{
    setAutoDelete(true);
    this->vol = vol;
}

void InitVolumeWork::run()
{
    Volume *volume = new Volume(vol);
    volume->BuildIndex();
    qDebug()<<"Inited: "<<vol; //TO DEL
    emit finished(volume);
}

FindWork::FindWork(Volume *volume, QString filename)
{
    setAutoDelete(true);
    this->volume = volume;
    this->filename = filename;
}

void FindWork::run()
{
    vector<SearchResultFile> result;
    volume->Find(filename.toStdWString(), &result);
    emit finished(filename, result);
}

FileData::FileData()
{

}

FileData::~FileData()
{
    for(Volume* volume: volumes)
    {
        delete volume;
    }
}

bool FileData::initVolumes()
{
    initValidVols();
    for (int i=0; i<vols.size(); ++i)
    {
        InitVolumeWork* work = new InitVolumeWork(vols.at(i));
        connect(work, &InitVolumeWork::finished, this, &FileData::onInitVolumeWorkFinished);
        QThreadPool::globalInstance()->start(work);
    }
    return true;
}

void FileData::findFile(QString filename)
{
    if(waitingInit | findingName == filename) return;
    if(filename.isEmpty())
    {
        findingName = filename;
        return;
    }
    findingName = filename;
    findingResult.clear();
    waitingFinder = volumes.length();

    for(Volume* volume: volumes)
    {
        FindWork* work = new FindWork(volume, findingName);
        connect(work, &FindWork::finished, this, &FileData::onFindWorkFinished);
        QThreadPool::globalInstance()->start(work);
    }
}

void FileData::onInitVolumeWorkFinished(Volume *volume)
{
    volumes.append(volume);
    waitingInit--;
}

void FileData::onFindWorkFinished(QString filename,vector<SearchResultFile> result)
{
    if( filename != this->findingName) return;
    findingResult.insert(findingResult.end(),result.begin(),result.end());
    if(--waitingFinder == 0){
        sort(findingResult.begin(), findingResult.end());
        emit updateSearchResult(filename, findingResult);
    }
}

void FileData::initValidVols()
{
    vols.empty();
    for (int i=0; i<26; ++i )
    {
        char cvol = i+'A';
        if ( isNTFS(cvol) ) vols.append(cvol);
    }
    waitingInit = vols.length();
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

    if ( GetVolumeInformationA(
        lpRootPathName,
        lpVolumeNameBuffer,
        MAX_PATH,
        &lpVolumeSerialNumber,
        &lpMaximumComponentLength,
        &lpFileSystemFlags,
        lpFileSystemNameBuffer,
        MAX_PATH
        )) {
        if (!strcmp(lpFileSystemNameBuffer, "NTFS")) {
            return true;
        }
    }
    return false;
}
