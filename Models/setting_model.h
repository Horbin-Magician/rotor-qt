#ifndef SETTINGMODEL_H
#define SETTINGMODEL_H

#include<QSettings>
#include<QStringList>


class SettingModel
{
public:
    static SettingModel& getInstance();

    bool setConfig(QString flag, QVariant value);
    QVariant getConfig(QString flag);

    QString Flag_IfPowerBoot = "IfPowerBoot";
    QString Flag_IgnoredPath = "IgnoredPath";
    QString Flag_Version = "Version";
    QStringList getIgnoredPath();
    void setIgnoredPath(QString);
private:
    explicit SettingModel(QObject * parent = nullptr);
    ~SettingModel();

    void initConfig();

    QSettings* m_Settings;

    QString version;
};

#endif // SETTINGMODEL_H
