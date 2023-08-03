#include "searcher.h"


Searcher::Searcher(QWidget *parent): QWidget(parent){
    m_fileData = nullptr;

    Searcher::initFileData();
    Searcher::initUI();
}

// init filedata and its volumes
void Searcher::initFileData(){
    if(m_fileData != nullptr && m_fileData->state != 2) return;
    delete m_fileData;
    m_fileData = new FileData();
    m_fileData->initVolumes();
    connect(m_fileData, &FileData::sgn_updateSearchResult, this, &Searcher::onSearchResultUpdate);
}

void Searcher::initUI(){
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::Popup);

    m_initialWidth = 570;
    m_initialHeight = 60;
    setFixedWidth(m_initialWidth);
    setMinimumHeight(m_initialHeight);

    QScreen *screen = QApplication::primaryScreen();
    QRect rect = screen->geometry();
    this->move((rect.width() - m_initialWidth) / 2, (rect.height() - m_initialHeight) / 3);
    this->setStyleSheet("background-color:rgb(221,221,221);");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(5);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setFixedSize(570, 60);
    m_lineEdit->setContentsMargins(20, 0, 20, 0);
    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_lineEdit->setStyleSheet("QLineEdit{font-size:28px; border:0px;}");
    connect(m_lineEdit, &QLineEdit::textChanged, this, &Searcher::onTextChanged);//点击托盘，执行相应的动作
    m_layout->addWidget(m_lineEdit);

    m_searchResultList = new SearchResultList(this);
    m_searchResultList->hide();
    m_layout->addWidget(m_searchResultList);
}

bool Searcher::event(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if(QApplication::activeWindow() != this) switchShow();
    }
    // if key press
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;

        if (keyEvent->key() == Qt::Key_Escape){ // Esc, hide
            hide();
        }
        else if(keyEvent->key() == Qt::Key_Return){ // Enter, open file
            m_searchResultList->openCurrent();
            switchShow();
        }
        else if(keyEvent->key() == Qt::Key_Up){ // Up, previous file
            m_searchResultList->up();
        }
        else if(keyEvent->key() == Qt::Key_Down){ // Down, next file
            m_searchResultList->down();
        }
    }
    return QWidget::event(event);
}

void Searcher::onHotkey(unsigned int fsModifiers, unsigned int  vk)
{
    switchShow();
}

void Searcher::switchShow(){
    if(this->isVisible()){
        this->hide();
        m_fileData->releaseIndex();
        m_searchResultList->release();
    }
    else{
        show();
        m_fileData->updateIndex();
        m_lineEdit->activateWindow();
        m_lineEdit->clear();
    }
}

void Searcher::onTextChanged(const QString &text){
    if( text.isEmpty() ){
        setFixedHeight(m_initialHeight);
        m_searchResultList->hide();
        m_lineEdit->setStyleSheet("QLineEdit{font-size:28px; border:0px;}");
    }
    m_fileData->findFile(text);
}

void Searcher::onSearchResultUpdate(const QString filename, const vector<SearchResultFile> &filepaths){
    if(m_lineEdit->text() != filename) return;
    m_searchResultList->update(filepaths);
    setFixedHeight(m_initialHeight + m_searchResultList->height());
}
