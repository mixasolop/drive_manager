#pragma once
#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>
#include <QJniObject>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>

class Backend : public QObject
{
    Q_OBJECT

private:
    QNetworkAccessManager m_manager;
    QString m_accestoken;
    QString m_refreshtoken;
public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent) {
        qDebug() << "Backend initialized";
    }

    Q_INVOKABLE void startAuth() {
        QJniObject::callStaticMethod<void>(
            "com/example/untitled/QtBridge",
            "startLogin",
            "()V"
            );

    }

    Q_INVOKABLE QString getAccessToken() {
        QJniObject token = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "getAccessToken",
            "()Ljava/lang/String;");
        return token.toString();
    }

    Q_INVOKABLE QString getRefreshToken() {
        QJniObject token = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "getRefreshToken",
            "()Ljava/lang/String;");
        return token.toString();
    }

    Q_INVOKABLE void printAccess(){
        qDebug() << "HERE IS THE ACCESS TOKEN: " << m_accestoken;
    }

    Q_INVOKABLE void printTokens() {
        m_accestoken = getAccessToken();
        m_refreshtoken = getRefreshToken();
        qDebug() << "ACCESS =" << m_accestoken;
        qDebug() << "REFRESH =" << m_refreshtoken;
    }





    Q_INVOKABLE void listfiles(){
        QUrl url("https://www.googleapis.com/drive/v3/files");
        QUrlQuery q;
        q.addQueryItem("pageSize", "999");
        q.addQueryItem("fields", "files(id, name)");

        url.setQuery(q);

        QNetworkRequest req(url);
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_accestoken).toUtf8());


        QNetworkReply *reply = m_manager.get(req);
        connect(reply, &QNetworkReply::finished, this, [reply]() {
            if(reply->error() != QNetworkReply::NoError){
                qDebug() << "Error!----------------------" << reply->errorString();
            }
            else{
                qDebug() << "HERE ARE THE FILES:" << reply->readAll();
            }
            reply->deleteLater();
        });
    }
    QByteArray readFileBytes(const QString &uri) {
        QJniObject juri = QJniObject::fromString(uri);

        QJniObject bytesObj = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "readFileBytes",
            "(Ljava/lang/String;)[B",
            juri.object<jstring>()
            );

        if (!bytesObj.isValid()) {
            qWarning() << "Failed to read bytes from Java";
            return {};
        }

        QJniEnvironment env;
        jbyteArray arr = bytesObj.object<jbyteArray>();
        jsize len = env->GetArrayLength(arr);

        QByteArray buffer;
        buffer.resize(len);

        env->GetByteArrayRegion(arr, 0, len, reinterpret_cast<jbyte*>(buffer.data()));
        return buffer;
    }

    QString getFileName(const QString &uri) {
        QJniObject juri = QJniObject::fromString(uri);

        QJniObject nameObj = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "getFileName",
            "(Ljava/lang/String;)Ljava/lang/String;",
            juri.object<jstring>()
            );

        return nameObj.toString();
    }

    QString getMimeType(const QString &uri) {
        QJniObject juri = QJniObject::fromString(uri);

        QJniObject mimeObj = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "getMimeType",
            "(Ljava/lang/String;)Ljava/lang/String;",
            juri.object<jstring>()
            );

        return mimeObj.toString();
    }


    Q_INVOKABLE void onFilesSelected(const QStringList &paths)
    {
        for (const QString &p : paths)
            uploadFileToDrive(p);
    }

    Q_INVOKABLE void uploadFileToDrive(const QString &uri) {
        QString fileName = getFileName(uri);
        QString mimeType = getMimeType(uri);
        QByteArray fileData = readFileBytes(uri);

        if (fileData.isEmpty())
            return;

        QByteArray meta = QByteArray("{\"name\": \"") + fileName.toUtf8() + "\"}";
        QString boundary = "foo_bar_baz";

        QByteArray body;
        body += "--" + boundary.toUtf8() + "\r\n";
        body += "Content-Type: application/json; charset=UTF-8\r\n\r\n";
        body += meta + "\r\n";
        body += "--" + boundary.toUtf8() + "\r\n";
        body += "Content-Type: " + mimeType.toUtf8() + "\r\n\r\n";
        body += fileData + "\r\n";
        body += "--" + boundary.toUtf8() + "--";

        QNetworkRequest req(QUrl("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"));
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_accestoken).toUtf8());
        req.setRawHeader("Content-Type", QString("multipart/related; boundary=%1").arg(boundary).toUtf8());
        req.setHeader(QNetworkRequest::ContentLengthHeader, body.size());

        QNetworkReply *reply = m_manager.post(req, body);

        connect(reply, &QNetworkReply::finished, this, [reply]() {
            QByteArray resp = reply->readAll();
            qDebug() << resp;
            reply->deleteLater();
        });
    }



    Q_INVOKABLE void pickFiles() {
        QJniObject::callStaticMethod<void>(
            "com/example/untitled/QtBridge",
            "pickFiles",
            "()V"
            );
    }

};
