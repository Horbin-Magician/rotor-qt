#ifndef SETTINGMODEL_H
#define SETTINGMODEL_H

#include<QSettings>
#include<QStringList>


class SettingModel
{
public:
    static SettingModel& getInstance();

    bool getIfPowerBoot();
    void setIfPowerBoot(bool value);
    QStringList getIgnoredPath();
    void setIgnoredPath(QString value);
private:
    explicit SettingModel(QObject * parent = nullptr);
    ~SettingModel();

    void initConfig();

    QSettings* m_Settings;
};

#endif // SETTINGMODEL_H
