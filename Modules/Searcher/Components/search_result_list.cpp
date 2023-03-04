#include "search_result_list.h"
#include "rotor.h"

SearchResultList::SearchResultList(QWidget *parent): QListWidget(parent)
{
    initUI();
    connect(this, &QListWidget::itemClicked, this, &SearchResultList::openCurrent);
}

void SearchResultList::update(const vector<SearchResultFile> &results)
{
    if(!isVisible())show();
    m_fileInfos.clear();

    int len = results.size();
    setFixedHeight( (60 * len > 540) ? 540 : 60 * len);

    for(int i =0; i < 20; ++i){
        QListWidgetItem* item = this->item(i);
        if (i < len){
            QString filename = results[i].filename;
            QString path = results[i].path;
            QFileInfo fileInfo( path + filename );
            m_fileInfos.append(fileInfo);
            if(item == nullptr){
                item = new QListWidgetItem(this);
                item->setSizeHint(QSize(570, 60));
                this->addItem(item);

                SearchResultItemWidget* widget = new SearchResultItemWidget(fileInfo);
                this->setItemWidget(item, widget);
            }else{
                SearchResultItemWidget* widget = (SearchResultItemWidget*)this->itemWidget(item);
                widget->update(fileInfo);
            }
            item->setHidden(false);
        } else {
            if(item != nullptr) item->setHidden(true);
        }
    }
    this->setCurrentRow(0);
}

// select previouse item when press up
void SearchResultList::up()
{
    int currentRow = this->currentRow();
    if(currentRow > 0) this->setCurrentRow(currentRow-1);;
}

// select next item when press down
void SearchResultList::down()
{
    int currentRow = this->currentRow();
    if(currentRow + 1 < m_fileInfos.length()) this->setCurrentRow(currentRow + 1);;
}

// open selected item
void SearchResultList::openCurrent()
{
    int currentRow = this->currentRow();
    if(currentRow < 0 || m_fileInfos.length() - 1 < currentRow ) return;
    QFileInfo fileInfo = m_fileInfos[currentRow];
    if( !QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.filePath())) ){
        Rotor::getInstance().showMessage("提示","无法打开该类型文件！", QSystemTrayIcon::MessageIcon::Information,2000);
    }
}

// open selected item's path
void SearchResultList::openCurrentPath()
{
    QFileInfo fileInfo = m_fileInfos[this->currentRow()];
    if( !QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath())) ){
        Rotor::getInstance().showMessage("提示","无法打开该文件夹！", QSystemTrayIcon::MessageIcon::Information,2000);
    }
}

void SearchResultList::initUI()
{
    this->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setStyleSheet("QListWidget{outline: none; border: none;}"
                        "QListWidget::item:selected{background: rgb(255,255,255);}"
                        "QListWidget::item:hover{background: rgb(200,200,200);}");
    m_ContextMenu = new QMenu(this);
    QAction* openPath = new QAction("打开文件所在目录", this);
    m_ContextMenu->addAction(openPath);
    connect(openPath, &QAction::triggered, this, &SearchResultList::openCurrentPath);
    connect(this, &SearchResultList::customContextMenuRequested,this, [&](){m_ContextMenu->exec(QCursor::pos());});
}

SearchResultItemWidget::SearchResultItemWidget(QFileInfo &fileInfo)
{
    m_icon = new QLabel();
    m_icon->setFixedSize(32, 32);

    QVBoxLayout* layout_right = new QVBoxLayout();
    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet("QLabel{font-size:14px;} ");
    layout_right->addWidget(m_titleLabel);
    m_subLabel = new QLabel();
    m_subLabel->setStyleSheet("QLabel{color: grey;}");
    layout_right->addWidget(m_subLabel);

    QHBoxLayout* layout_main = new QHBoxLayout();
    layout_main->addWidget(m_icon);
    layout_main->addLayout(layout_right);
    setLayout(layout_main);
    setStyleSheet("*{background:transparent;} .QWidget{padding:10px}");
    update(fileInfo);
}

void SearchResultItemWidget::update(QFileInfo &fileInfo)
{
    if(fileInfo.isShortcut()) m_pixmap = m_iconProvider.icon(QFileInfo(fileInfo.symLinkTarget())).pixmap(32, 32);
    else m_pixmap = m_iconProvider.icon(fileInfo).pixmap(32, 32);
    m_icon->setPixmap(m_pixmap);

    m_titleLabel->setText(fileInfo.fileName());
    m_subLabel->setText(fileInfo.absolutePath());
}
