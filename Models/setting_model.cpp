#include "setting_model.h"
#include "Utils/powerboot.h"

SettingModel::SettingModel(QObject *parent)
{
    this->version = "1.4.0";

    QString appPath = QApplication::applicationDirPath(); // get programe path
    m_Settings = new QSettings(appPath + "/userdata/config.ini", QSettings::IniFormat);
    QStringList stringList = m_Settings->allKeys();
    // if no config, init config
    if(stringList.size() == 0){
        this->initConfig();
    }
}

SettingModel::~SettingModel()
{
    m_Settings->sync();
}

SettingModel &SettingModel::getInstance()
{
    static SettingModel m_pInstance;
    return m_pInstance;
}

bool SettingModel::setConfig(const QString &flag, const QVariant &value)
{
    m_Settings->setValue(flag, value);
    if(flag == Flag_IfPowerBoot) PowerBoot::setProcessAutoRun(value.toBool());
    return true;
}

QVariant SettingModel::getConfig(QString flag)
{
    if(flag == Flag_Version) return QVariant(this->version);
    else return m_Settings->value(flag);
}

// init config when no config exit
void SettingModel::initConfig()
{
    setConfig(Flag_IfPowerBoot, QVariant(false));
    setConfig(Flag_Save_Path, QVariant(""));
}
