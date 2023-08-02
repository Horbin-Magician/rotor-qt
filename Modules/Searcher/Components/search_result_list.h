#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <QListWidget>
#include <QMenu>
#include <QFileInfo>
#include <QPixmap>
#include <QFileIconProvider>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>

#include "../FileData/volume.h"

class SearchResultItemWidget: public QWidget
{
    Q_OBJECT
public:
    SearchResultItemWidget(QFileInfo &fileInfo);
    void update(QFileInfo &fileInfo);
private:
    QFileIconProvider m_iconProvider;
    QPixmap m_pixmap;
    QLabel* m_icon;
    QLabel* m_titleLabel;
    QLabel* m_subLabel;
};

class SearchResultList: public QListWidget
{
    Q_OBJECT
public:
    SearchResultList(QWidget *parent);
    void update(const vector<SearchResultFile> &results);
    void clear();
    void up();
    void down();
    void openCurrent();
    void openCurrentAdmin();
    void openCurrentPath();

    void release();
private:
    void initUI();
    QMenu* m_ContextMenu;
    QList<QFileInfo> m_fileInfos;
    QProcess m_process;
};

#endif // SEARCHRESULTLIST_H
