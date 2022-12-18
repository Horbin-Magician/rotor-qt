#ifndef FILEDATA_H
#define FILEDATA_H

#include <QList>
#include <QObject>
#include <QRunnable>
#include <QtConcurrent/QtConcurrent>

#include "volume.h"


class InitVolumeWork:public QObject, public QRunnable
{
    Q_OBJECT
signals:
    void finished(Volume *volume);
public:
    explicit InitVolumeWork(char vol);
    void run() override;
private:
    char m_vol;
};

class FindWork:public QObject, public QRunnable
{
    Q_OBJECT
signals:
    void finished(QString filename, vector<SearchResultFile>* filepaths);
public:
    explicit FindWork(Volume *volume, QString filename);
    void run() override;
    void stop();
private:
    Volume *m_volume;
    QString m_filename;
};

class FileData:public QObject
{
    Q_OBJECT
signals:
    void updateSearchResult(QString filename, vector<SearchResultFile> filepaths);
    void stopFind();
public:
    FileData();
    ~FileData();
    bool initVolumes();
    void findFile(QString filename);
    void updateIndex();

    unsigned short state; // 0, created; 1, initing; 2, inited.

private:
    QList<char> m_vols;
    QList<Volume*> m_volumes;
    QList<QFuture<void>> m_futures;

    QString m_findingName;
    vector<SearchResultFile> m_findingResult;

    unsigned short m_waitingFinder;
    unsigned short m_waitingInit;

    unsigned short initValidVols();
    bool isNTFS(char vol);

    void onInitVolumeWorkFinished(Volume* volume);
    void onFindWorkFinished(QString filename, vector<SearchResultFile>* result);
};

#endif // FILEDATA_H
