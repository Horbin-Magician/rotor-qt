#ifndef SETTING_H
#define SETTING_H

#include "Parents/w_normalwindow.h"
#include "../Model/settingmodel.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QList>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>

//enum SETTINGTYPE{CHECK_BOX};

//struct SettingItem{
//    SETTINGTYPE type;
//    QString string1;
//    QString string2;
//    QString value;
//};

//struct SettingPage{
//    QString settingName;
//    QList<SettingItem> itemList;
//};

class Setting : public QWidget
{
    Q_OBJECT
public:
    explicit Setting(QWidget *parent = nullptr);
    bool SwitchContent(short index);

private:
    QList<QString> m_SettingNames;
    QHBoxLayout* m_MainLayout;
    QListWidget* m_LeftWidget;
    QStackedWidget* m_RightWidget;

    void initUI();
    void initSettingWidgets();

signals:
    void rebuildIndex();
};

#endif // SETTING_H
