#ifndef SETTINGMODEL_H
#define SETTINGMODEL_H

#include<QSettings>
#include<QStringList>


class SettingModel
{
public:
    static SettingModel& getInstance();

    bool setConfig(const QString &flag, const QVariant &value);
    QVariant getConfig(QString flag);

    // 新增配置只需要
    // 1、在这里添加flag；
    // 2、在initConfig()中添加初始值；
    // 3、在setConfig添加设置时的事件。
    QString Flag_IfPowerBoot = "IfPowerBoot";
    QString Flag_Version = "Version";
    QString Flag_Save_Path = "SavePath";
private:
    explicit SettingModel(QObject * parent = nullptr);
    ~SettingModel();

    void initConfig();

    QSettings* m_Settings;
    QString version;
};

#endif // SETTINGMODEL_H
