#include "setting_model.h"
#include "Utils/powerboot.h"

SettingModel::SettingModel(QObject *parent)
{
    QString appPath = QApplication::applicationDirPath(); // get programe path
    m_Settings = new QSettings(appPath + "/config.ini", QSettings::IniFormat);
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

bool SettingModel::getIfPowerBoot()
{
    return m_Settings->value("IfPowerBoot").toBool();
}

void SettingModel::setIfPowerBoot(bool value)
{
    PowerBoot::setProcessAutoRun(value);
    m_Settings->setValue("IfPowerBoot", QVariant(value));
}

QStringList SettingModel::getIgnoredPath()
{
    return m_Settings->value("IgnoredPath").toStringList();
}

void SettingModel::setIgnoredPath(QString value)
{
    QStringList list = value.split(';');
    m_Settings->setValue("IgnoredPath", QVariant(list));
}

// init config when no config exit
void SettingModel::initConfig()
{
    setIfPowerBoot(false);
    m_Settings->setValue("IgnoredPath", QVariant(QStringList("C:\\Windows")));
}
