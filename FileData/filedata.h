#ifndef INIT_VOLUME_WORK
#define INIT_VOLUME_WORK

#include "volume.h"

#include <QObject>
#include <QRunnable>

class InitVolumeWork:public QObject, public QRunnable
{
    Q_OBJECT

signals:
    void finished(Volume *volume);

public:
    explicit InitVolumeWork(char vol);
    void run() override;

private:
    char vol;
};

class FindWork:public QObject, public QRunnable
{
    Q_OBJECT

signals:
    void finished(QString filename, vector<SearchResultFile> filepaths);

public:
    explicit FindWork(Volume *volume, QString filename);
    void run() override;

private:
    Volume *volume;
    QString filename;
};


#endif // INIT_VOLUME_WORK


#ifndef SEARCH_WORK
#define SEARCH_WORK



#endif // SEARCH_WORK



#ifndef FILEDATA_H
#define FILEDATA_H

#include "volume.h"

#include <QList>


class FileData:public QObject
{
    Q_OBJECT
signals:
    void updateSearchResult(QString filename, vector<SearchResultFile> filepaths);
public:
    FileData();
    ~FileData();

    bool initVolumes();
    void findFile(QString filename);
protected:
    void onInitVolumeWorkFinished(Volume* volume);
    void onFindWorkFinished(QString filename, vector<SearchResultFile> result);
private:
    QList<char> vols;
    QList<Volume*> volumes;

    QString findingName;
    vector<SearchResultFile> findingResult;
    unsigned short waitingFinder;
    unsigned short waitingInit;

    void initValidVols();
    bool isNTFS(char vol);
};

#endif // FILEDATA_H
