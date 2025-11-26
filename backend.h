#pragma once
#include <QObject>
#include <QDebug>
#include <QString>
#include <QUrl>
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
};
