#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <FileData/volume.h>
#include <QListWidget>
#include <QMenu>
#include <QFileInfo>

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
    void onCustomContextMenuRequested(const QPoint &pos);
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    void initUI();
    QWidget* getWidgetItem(QFileInfo fileInfo);
    QMenu* m_ContextMenu;
    QList<QFileInfo> m_fileInfos;
};

#endif // SEARCHRESULTLIST_H
