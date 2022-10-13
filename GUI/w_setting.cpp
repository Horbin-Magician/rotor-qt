#include "w_setting.h"
#include "Model/setting_model.h"

Setting::Setting(QWidget *parent) : QWidget{parent}
{
    this->initUI();
}

bool Setting::SwitchContent(const short index)
{
    m_RightWidget->setCurrentIndex(index);
    return true;
}

void Setting::initUI()
{
    this->setWindowTitle("设置");
    this->setMinimumWidth(800);
    this->setMinimumHeight(500);

    // init left part
    m_LeftWidget = new QListWidget();
    m_LeftWidget->setFixedWidth(100);
    connect(m_LeftWidget, &QListWidget::currentRowChanged, this, &Setting::SwitchContent);
    // init right part
    m_RightWidget = new QStackedWidget();
    m_SettingNames = QList<QString>() << "常规" << "搜索";
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
    for(int i = 0; i < m_SettingNames.size(); i++)
    {
        QWidget* widget = m_RightWidget->widget(i);
        if(m_SettingNames[i] == "常规"){
            QCheckBox* ifPowerBootCheckBox = new QCheckBox();
            ifPowerBootCheckBox->setText("开机启动");
            ifPowerBootCheckBox->setChecked(settingModel.getIfPowerBoot());
            connect(ifPowerBootCheckBox, &QCheckBox::stateChanged, [&settingModel](int state){
                settingModel.setIfPowerBoot(state);
            });
            widget->layout()->addWidget(ifPowerBootCheckBox);
        }
        else if(m_SettingNames[i] == "搜索"){
            QWidget* filterWidget = new QWidget();
            QHBoxLayout* filterLayout = new QHBoxLayout();
            filterWidget->setLayout(filterLayout);
            QLabel* filterLabel = new QLabel();
            filterLabel->setText("忽略目录：");
            filterLayout->addWidget(filterLabel);
            QLineEdit* lineEdit = new QLineEdit();
            lineEdit->setText(settingModel.getIgnoredPath().join(';'));
            connect(lineEdit, &QLineEdit::textChanged, [&](const QString &text){
                settingModel.setIgnoredPath(text);
            });
            filterLayout->addWidget(lineEdit);
            widget->layout()->addWidget(filterWidget);

            QPushButton* resetButton = new QPushButton();
            resetButton->setText("重建索引");
            connect(resetButton, &QPushButton::clicked, [&](){emit rebuildIndex();});
            widget->layout()->addWidget(resetButton);
        }
        ( (QBoxLayout*)( widget->layout() ) )->addStretch();
    }
}
