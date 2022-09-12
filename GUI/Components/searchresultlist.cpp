#include "searchresultlist.h"

#include <QPixmap>
#include <QFileIconProvider>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QLabel>

#include "manager.h"


SearchResultList::SearchResultList(QWidget *parent): QListWidget(parent)
{
    initUI();
    connect(this, &QListWidget::itemClicked, this, &SearchResultList::openCurrent);
    connect(this, &QListWidget::currentItemChanged, this, &SearchResultList::onCurrentItemChanged);
    hide();
}

void SearchResultList::update(const vector<SearchResultFile> &results)
{
    if(!isVisible())show();
    QListWidget::clear();
    m_fileInfos.clear();

    int len = results.size();
     setFixedHeight( (60 * len > 540) ? 540 : 60 * len);
     for(int i =0; i < (len > 20 ? 20 : len); i++)
     {
         QString filename = QString::fromStdWString(results[i].filename);
         QString path = QString::fromStdWString(results[i].path);
         QFileInfo fileInfo( path + filename);

         QListWidgetItem *item = new QListWidgetItem(this);
         item->setSizeHint(QSize(570, 60));

         QWidget* widget = getWidgetItem(fileInfo);
         m_fileInfos.append(fileInfo);

         this->addItem(item);
         this->setItemWidget(item, widget);
     }
     this->setCurrentRow(0);
}

void SearchResultList::up()
{
    int currentRow = this->currentRow();
    if(currentRow > 0) this->setCurrentRow(currentRow-1);;
}

void SearchResultList::down()
{
    int currentRow = this->currentRow();
    if(currentRow + 1 < this->count()) this->setCurrentRow(currentRow + 1);;
}

void SearchResultList::openCurrent()
{
    if(this->currentRow() < 0) return;
    QFileInfo fileInfo = m_fileInfos[this->currentRow()];
    if( !QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.filePath())) ){
        Manager::getInstance().showMessage("提示","无法打开该类型文件！", QSystemTrayIcon::MessageIcon::Information,2000);
    }
}

void SearchResultList::openCurrentPath()
{
    QFileInfo fileInfo = m_fileInfos[this->currentRow()];
    if( !QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath())) ){
        Manager::getInstance().showMessage("提示","无法打开该文件夹！", QSystemTrayIcon::MessageIcon::Information,2000);
    }
}

void SearchResultList::onCustomContextMenuRequested(const QPoint &pos)
{
    m_ContextMenu->exec(QCursor::pos());
}

void SearchResultList::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QWidget* previousWidget = this->itemWidget(previous);
    QWidget* currentWidget = this->itemWidget(current);
    if(previousWidget) previousWidget->setStyleSheet("QWidget{background:transparent;} .QWidget:hover{background:rgb(255, 255, 255);}");
    if(currentWidget) currentWidget->setStyleSheet("QWidget{background:rgb(255, 255, 255);}");
}

void SearchResultList::initUI()
{
    this->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setStyleSheet("QListWidget{outline: none; border: none;}");

    m_ContextMenu = new QMenu(this);
    QAction* openPath = new QAction("打开文件所在目录", this);
    m_ContextMenu->addAction(openPath);
    connect(openPath, &QAction::triggered, this, &SearchResultList::openCurrentPath);
    connect(this, &SearchResultList::customContextMenuRequested,this, &SearchResultList::onCustomContextMenuRequested);
}

QWidget* SearchResultList::getWidgetItem(QFileInfo fileInfo)
{
    QFileIconProvider iconProvider;
    QPixmap pixmap;
    if(fileInfo.isShortcut()) pixmap = iconProvider.icon(QFileInfo(fileInfo.symLinkTarget())).pixmap(32, 32);
    else pixmap = iconProvider.icon(fileInfo).pixmap(32, 32);

    QWidget* widget = new QWidget();
    widget->setStyleSheet(".QWidget{padding:10px} QWidget{background:transparent;} .QWidget:hover{background:rgb(255, 255, 255);}");
    QHBoxLayout* layout_main = new QHBoxLayout();

    QLabel* icon = new QLabel();
    icon->setFixedSize(32, 32);
    icon->setPixmap(pixmap);

    QVBoxLayout* layout_right = new QVBoxLayout();
    QLabel* titleLabel = new QLabel(fileInfo.fileName());
    titleLabel->setStyleSheet("QLabel{font-size:14px;}");
    QLabel* subLabel = new QLabel(fileInfo.absolutePath());
    subLabel->setStyleSheet("QLabel{color: grey;}");
    layout_right->addWidget(titleLabel);
    layout_right->addWidget(subLabel);

    layout_main->addWidget(icon);
    layout_main->addLayout(layout_right);
    widget->setLayout(layout_main);

    return widget;
}
