#include "w_setting.h"
#include "Models/setting_model.h"
#include "Utils/commonUtils.h"


Setting::Setting(QWidget *parent) : QWidget{parent}
{
    this->initUI();
    setAttribute(Qt::WA_DeleteOnClose, false);

}

bool Setting::SwitchContent(const short index)
{
    m_RightWidget->setCurrentIndex(index);
    return true;
}

void Setting::initUI()
{
    this->setWindowTitle("设置");
    this->setFixedWidth(500);
    this->setFixedHeight(300);

    // init left part
    m_LeftWidget = new QListWidget();
    m_LeftWidget->setFixedWidth(70);
    connect(m_LeftWidget, &QListWidget::currentRowChanged, this, &Setting::SwitchContent);
    // init right part
    m_RightWidget = new QStackedWidget();
    m_SettingNames = QList<QString>() << "常规" << "搜索"<< "关于";
    // add widgets to right widget by setting names
    for(int i = 0; i < m_SettingNames.size(); i++){
        m_LeftWidget->addItem(m_SettingNames[i]);
        QWidget* settingWidget = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout();
        settingWidget->setLayout(layout);
        m_RightWidget->addWidget(settingWidget);
    }
    // init main part
    m_MainLayout = new QHBoxLayout();
    m_MainLayout->setSpacing(0);
    this->setLayout(m_MainLayout);
    m_MainLayout->addWidget(m_LeftWidget);
    m_MainLayout->addWidget(m_RightWidget);

    this->initSettingWidgets(); // init content of right part
    // select default
    m_LeftWidget->setCurrentRow(0);
    this->SwitchContent(0);
}

void Setting::initSettingWidgets()
{
    SettingModel& settingModel = SettingModel::getInstance();
    m_version = settingModel.getConfig(settingModel.Flag_Version).toString();

    for(int i = 0; i < m_SettingNames.size(); i++)
    {
        QWidget* widget = m_RightWidget->widget(i);
        if(m_SettingNames[i] == "常规"){
            QCheckBox* ifPowerBootCheckBox = new QCheckBox();
            ifPowerBootCheckBox->setText("开机启动");
            ifPowerBootCheckBox->setChecked( settingModel.getConfig(settingModel.Flag_IfPowerBoot).toBool() );
            connect(ifPowerBootCheckBox, &QCheckBox::stateChanged, [&settingModel](int state){
                settingModel.setConfig(settingModel.Flag_IfPowerBoot, QVariant(state));
            });
            widget->layout()->addWidget(ifPowerBootCheckBox);
        }
        else if(m_SettingNames[i] == "搜索"){

        }
        else if(m_SettingNames[i] == "关于"){
            QLabel* aboutLabel = new QLabel("Rotor 版本：" + m_version);
            QPushButton* pushButton = new QPushButton("检查更新");
            connect(pushButton, &QPushButton::clicked, [&](){
                Updater* updater = new Updater();
                updater->InspectUpdate(m_version);
            });
            widget->layout()->addWidget(aboutLabel);
            widget->layout()->addWidget(pushButton);
        }
        ( (QBoxLayout*)( widget->layout() ) )->addStretch();
    }
}
