#ifndef SEARCHER_H
#define SEARCHER_H

#include <windows.h>
#include <QtWidgets/QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QKeyEvent>

#include "../i_module.h"
#include "FileData/file_data.h"
#include "Components/search_result_list.h"


class Searcher : public QWidget, IModule
{
    Q_OBJECT
public:
    Searcher(QWidget *parent = 0);

    void SwitchShow();
    void initFileData();
    virtual void onHotkey();

public slots:
    bool eventFilter(QObject *obj, QEvent * event);

protected:
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
