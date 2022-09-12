#include "w_searcher.h"


Searcher::Searcher(QWidget *parent): QWidget(parent)
{
    m_fileData = nullptr;
    Searcher::initFileData();
    Searcher::initUI();
    //注册热键
    RegisterHotKey((HWND)this->winId(),1,MOD_SHIFT, (UINT)0x46);
}

Searcher::~Searcher()
{
    //注销热键
    UnregisterHotKey((HWND)this->winId(),1);
}

void Searcher::show()
{
    QWidget::show();
    m_lineEdit->activateWindow();
    m_lineEdit->clear();
}

bool Searcher::eventFilter(QObject *obj, QEvent * event)
{
    if (Q_NULLPTR == obj) return false;
    // 识别焦点丢失
    if (QEvent::ActivationChange == event->type()) {
        if(QApplication::activeWindow() != this){
            this->hide();
        }
    }
    // 识别按键
    if (event->type() == QEvent::KeyPress)
    {
       QKeyEvent *keyEvent = (QKeyEvent*)event;
       if (keyEvent->key() == Qt::Key_Escape) this->hide();                                                   // 按下Esc时隐藏窗口
       else if(keyEvent->key() == Qt::Key_Return) m_searchResultList->openCurrent();      // 按下Enter时打开文件
       else if(keyEvent->key() == Qt::Key_Up) m_searchResultList->up();                            // 按下Up时上选文件
       else if(keyEvent->key() == Qt::Key_Down) m_searchResultList->down();                   // 按下Down时下选文件
   }
   return false;


    return QWidget::eventFilter(obj, event);
}

bool Searcher::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if(eventType == "windows_generic_MSG") {
         MSG *msg = static_cast<MSG *>(message);
         if(msg->message == WM_HOTKEY) {
             //处理热键消息
             UINT fuModifiers = (UINT) LOWORD(msg->lParam);
             UINT uVirtKey = (UINT) HIWORD(msg->lParam);

             if(fuModifiers == MOD_SHIFT & uVirtKey == (UINT)0x46)
             {
                 if(this->isVisible()) this->hide();
                 else this->show();
             }
         }
     }
    return QWidget::nativeEvent(eventType, message, result);
}

void Searcher::initFileData()
{
    if(m_fileData) delete m_fileData;
    m_fileData = new FileData();
    m_fileData->initVolumes();
    connect(m_fileData, &FileData::updateSearchResult, this, &Searcher::onSearchResultUpdate);
}

void Searcher::initUI()
{
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
    m_layout->addWidget(m_lineEdit);

    m_searchResultList = new SearchResultList(this);
    m_layout->addWidget(m_searchResultList);

    connect(m_lineEdit, &QLineEdit::textChanged, this, &Searcher::onTextChanged);//点击托盘，执行相应的动作
    connect(m_fileData, &FileData::updateSearchResult, this, &Searcher::onSearchResultUpdate);
    installEventFilter(this);
}

void Searcher::onTextChanged(const QString &text)
{
    if( text.isEmpty() ){
        setFixedHeight(m_initialHeight);
        m_searchResultList->hide();
        m_lineEdit->setStyleSheet("QLineEdit{font-size:28px; border:0px;}");
    }
    m_fileData->findFile(text);
}

void Searcher::onSearchResultUpdate(const QString filename, const vector<SearchResultFile> &filepaths)
{
    if(m_lineEdit->text() != filename) return;
    m_searchResultList->update(filepaths);
    setFixedHeight(m_initialHeight + m_searchResultList->height());
    m_lineEdit->setStyleSheet(m_lineEdit->styleSheet() + "QLineEdit{border-bottom:1px solid rgb(180,180,180);}");
}

