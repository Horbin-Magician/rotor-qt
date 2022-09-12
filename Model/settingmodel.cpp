#include "settingmodel.h"


SettingModel &SettingModel::getInstance()
{
    static SettingModel m_pInstance;
    return m_pInstance;
}

bool SettingModel::getIfPowerBoot()
{
    return m_Settings->value("IfPowerBoot").toBool();
}

QStringList SettingModel::getIgnoredPath()
{
    return m_Settings->value("IgnoredPath").toStringList();
}

void SettingModel::setIfPowerBoot(bool value)
{
    PowerBoot::setProcessAutoRun(value);
    m_Settings->setValue("IfPowerBoot", QVariant(value));
}

void SettingModel::setIgnoredPath(QString value)
{
    qDebug()<<2;
    QStringList list = value.split(';');
    m_Settings->setValue("IgnoredPath", QVariant(list));
    qDebug()<<3;
}

SettingModel::SettingModel(QObject *parent)
{
    //取程序当前工作目录
    QString appPath = QApplication::applicationDirPath();
    m_Settings = new QSettings(appPath + "/config.ini", QSettings::IniFormat);
    QStringList stringList = m_Settings->allKeys();
    if(stringList.size() == 0){
        this->init();
    }
}

SettingModel::~SettingModel()
{
    m_Settings->sync();
}

void SettingModel::init()
{
    setIfPowerBoot(false);
    m_Settings->setValue("IgnoredPath", QVariant(QStringList("C:\\Windows")));
}
