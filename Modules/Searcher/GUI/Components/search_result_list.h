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
#include <QUrl>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QLabel>

#include "../../FileData/volume.h"

class SearchResultItemWidget: public QWidget
{
    Q_OBJECT
public:
    SearchResultItemWidget(QFileInfo fileInfo);
    void update(QFileInfo fileInfo);
private:
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
    void openCurrentPath();
private:
    void initUI();
    QMenu* m_ContextMenu;
    QList<QFileInfo> m_fileInfos;
};

#endif // SEARCHRESULTLIST_H
