#ifndef SEARCHER_H
#define SEARCHER_H

#include "FileData/filedata.h"
#include "Components/searchresultlist.h"

#include <windows.h>
#include <QtWidgets/QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QKeyEvent>


class Searcher : public QWidget
{
    Q_OBJECT
public:
    Searcher(QWidget *parent = 0);
    ~Searcher();

    void show();
    void initFileData();
public slots:
    bool eventFilter(QObject *obj, QEvent * event);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
    void onTextChanged(const QString &text);
    void onSearchResultUpdate(const QString filename, const vector<SearchResultFile> &filepaths);

private:
    void initUI();

    FileData *m_fileData;
    SearchResultList *m_searchResultList;

    QVBoxLayout *m_layout;
    QLineEdit *m_lineEdit;

    unsigned int m_initialWidth = 570;
    unsigned int m_initialHeight = 60;
};

#endif // SYSTEMTRAY_H
