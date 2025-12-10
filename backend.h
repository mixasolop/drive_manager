#pragma once
#include <QObject>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QJniObject>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <math.h>

class Backend : public QObject
{
    Q_OBJECT

signals:
    void loginSuccess();
    void loginFailed();
    void userAddFailed();
    void emailAddFailed();

private:
    QNetworkAccessManager m_manager;
    QString m_accestoken;
    QString m_refreshtoken;
    QString m_userName;
    QString m_userEmail;
    int curr_user_id = -1;

public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent) {
        qDebug() << "Backend initialized";
    }

    QByteArray generateSalt()
    {
        quint64 val = QRandomGenerator::system()->generate64();
        return QByteArray::number(val, 16);
    }

    QByteArray hashPassword(const QString &password, const QByteArray &salt) {
        return QCryptographicHash::hash(
                   (password.toUtf8() + salt),
                   QCryptographicHash::Sha3_512
                   ).toHex();
    }

    bool usernameExists(const QString &username)
    {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM users WHERE username = :u");
        q.bindValue(":u", username);
        q.exec();
        q.next();
        return q.value(0).toInt() > 0;
    }

    bool addUser(const QString &username, const QString &password)
    {
        if (usernameExists(username)) {
            qDebug() << "username already taken";
            emit userAddFailed();
            return false;
        }

        QByteArray salt = generateSalt();
        QByteArray passwordHash = hashPassword(password, salt);

        QSqlQuery query;
        query.prepare(R"(
            INSERT INTO users (username, salt, password_hash)
            VALUES (:username, :salt, :hash)
        )");

        query.bindValue(":username", username);
        query.bindValue(":salt", QString(salt));
        query.bindValue(":hash", QString(passwordHash));

        if (!query.exec()) {
            emit userAddFailed();
            return false;
        }

        return true;
    }

    int checkUser(const QString &username, const QString &password)
    {
        QSqlQuery query;
        query.prepare("SELECT id, salt, password_hash FROM users WHERE username = :u");
        query.bindValue(":u", username);
        query.exec();

        if (!query.next()) {
            emit loginFailed();
            return -1;
        }

        int id = query.value("id").toInt();
        QByteArray salt = query.value("salt").toByteArray();
        QByteArray hash = query.value("password_hash").toByteArray();

        QByteArray check = hashPassword(password, salt);

        if (check == hash) {
            curr_user_id = id;
            emit loginSuccess();
            return id;
        }

        emit loginFailed();
        return -1;
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

    Q_INVOKABLE QString getUserName() {
        QJniObject obj = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "getUserName",
            "()Ljava/lang/String;"
            );
        return obj.toString();
    }

    Q_INVOKABLE QString getUserEmail() {
        QJniObject obj = QJniObject::callStaticObjectMethod(
            "com/example/untitled/QtBridge",
            "getUserEmail",
            "()Ljava/lang/String;"
            );
        return obj.toString();
    }

    Q_INVOKABLE void onLoginFinished() {
        m_accestoken   = getAccessToken();
        m_refreshtoken = getRefreshToken();
        m_userName     = getUserName();
        m_userEmail    = getUserEmail();

        qDebug() << "[LOGIN COMPLETED]";
        qDebug() << "ACCESS ="  << m_accestoken;
        qDebug() << "REFRESH =" << m_refreshtoken;
        qDebug() << "NAME ="    << m_userName;
        qDebug() << "EMAIL ="   << m_userEmail;

        addUserEmail(curr_user_id, m_userEmail, m_accestoken, m_refreshtoken);
    }

    bool emailExists(int userId, const QString &email)
    {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM user_emails WHERE user_id = :id AND email = :email");
        q.bindValue(":id", userId);
        q.bindValue(":email", email);
        q.exec();
        q.next();
        return q.value(0).toInt() > 0;
    }

    bool addUserEmail(int userId, const QString &email,
                      const QString &accessToken,
                      const QString &refreshToken)
    {
        if (emailExists(userId, email)) {
            qDebug() << "email already exists!";
            emit emailAddFailed();
            return false;
        }

        QSqlQuery query;
        query.prepare(R"(
            INSERT INTO user_emails (user_id, email, access_token, refresh_token)
            VALUES (:uid, :email, :at, :rt)
        )");

        query.bindValue(":uid", userId);
        query.bindValue(":email", email);
        query.bindValue(":at", accessToken);
        query.bindValue(":rt", refreshToken);

        if (!query.exec()) {
            emit emailAddFailed();
            return false;
        }

        return true;
    }

    Q_INVOKABLE void printTokens(){
        qDebug() << "email: " << m_userEmail << "\nname: " << m_userName << "\naccess: " << m_accestoken << "\nrefresh: " << m_refreshtoken;
    }

    Q_INVOKABLE void listfiles() {
        QUrl url("https://www.googleapis.com/drive/v3/files");
        QUrlQuery q;
        q.addQueryItem("pageSize", "999");
        q.addQueryItem("fields", "files(id, name)");

        url.setQuery(q);

        QNetworkRequest req(url);
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_accestoken).toUtf8());

        QNetworkReply *reply = m_manager.get(req);
        connect(reply, &QNetworkReply::finished, this, [reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "Error!----------------------" << reply->errorString();
            } else {
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

        env->GetByteArrayRegion(arr, 0, len,
                                reinterpret_cast<jbyte*>(buffer.data()));
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
