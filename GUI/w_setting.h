#ifndef SETTING_H
#define SETTING_H

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


class Setting : public QWidget
{
    Q_OBJECT
public:
    explicit Setting(QWidget *parent = nullptr);
    bool SwitchContent(const short index);

private:
    QList<QString> m_SettingNames;
    QHBoxLayout* m_MainLayout;
    QListWidget* m_LeftWidget;
    QStackedWidget* m_RightWidget;
    QString m_version;

    void initUI();
    void initSettingWidgets();
};

#endif // SETTING_H
