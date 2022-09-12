#ifndef SETTINGMODEL_H
#define SETTINGMODEL_H

#include "Utils/powerboot.h"

#include<QSettings>
#include<QStringList>


class SettingModel
{
public:
    static SettingModel& getInstance();

    bool getIfPowerBoot();
    void setIfPowerBoot(bool value);

    void setIgnoredPath(QString value);
    QStringList getIgnoredPath();
private:
    explicit SettingModel(QObject * parent = nullptr);
    ~SettingModel();

    void init();

    QSettings* m_Settings;
};

#endif // SETTINGMODEL_H
