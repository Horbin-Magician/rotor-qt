#ifndef COMMONUTILS_H
#define COMMONUTILS_H


#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDesktopServices>

class Updater: public QObject
{
    Q_OBJECT
public:
    Updater() {}

    void InspectUpdate(QString version)
    {
        m_NetManager = new QNetworkAccessManager();
        QNetworkRequest request;
        m_nowVersion = version;

        request.setUrl(QUrl("https://api.github.com/repos/Horbin-Magician/Rotor/releases/latest"));
        m_UpdateMsg = m_NetManager->get(request);
        QObject::connect(m_NetManager, &QNetworkAccessManager::finished, this, &Updater::OnGetUpdateMsg);
    }

    void OnGetUpdateMsg()
    {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(m_UpdateMsg->readAll());

        if(jsonDoc.isObject()){
            QJsonObject obj = jsonDoc.object();
            QString latestVersion =  obj.value("tag_name").toString();
            if(latestVersion != "v" + m_nowVersion){
                QJsonArray assetsArray = obj.value("assets").toArray();
                QJsonValue assets = assetsArray.at(0);
                m_downloadURL = assets.toObject().value("browser_download_url").toString();

                int ret = QMessageBox::question(nullptr, "检查更新", "检查到新版本，是否下载？");
                if(ret == QMessageBox::Yes) QDesktopServices::openUrl(QUrl(m_downloadURL));
            }else{
                QMessageBox::information(nullptr, "检查更新", "当前已是最新版本，无需更新。");
            }
        }
    }
private:
    QNetworkReply* m_UpdateMsg;
    QNetworkAccessManager* m_NetManager;
    QString m_downloadURL;
    QString m_nowVersion;
};



#endif // COMMONUTILS_H
