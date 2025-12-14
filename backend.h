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
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <fstream>

class Backend : public QObject
{
    Q_OBJECT

signals:
    void loginSuccess();
    void loginFailed();
    void userAddFailed();
    void emailAddFailed();
    void fileUploadSuccess(QString fileName);

private:
    QNetworkAccessManager m_manager;
    QString m_accestoken;
    QString m_refreshtoken;
    QString m_userName;
    QString m_userEmail;
    QString username;
    QString password;
    int curr_user_id = -1;
    QString m_nextPageToken;
    int upload_curr_id = -1;

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

    Q_INVOKABLE bool addUser(const QString &username, const QString &password)
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
        qDebug() << "USER ADDED!!";
        return true;
    }

    Q_INVOKABLE int checkUser(const QString &username, const QString &password)
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

    Q_INVOKABLE void set_upload_accesstoken(int access){
        upload_curr_id = access;
    }

    Q_INVOKABLE void getuser(const QString t) {
        username = t;
        qDebug() << "username:" << username;
    }
    Q_INVOKABLE void getpass(const QString t) {
        password = t;
        qDebug() << "password:" << password;
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

    Q_INVOKABLE QStringList getUserEmails()
    {
        QStringList emails;

        if (curr_user_id < 0)
            return emails;

        QSqlQuery q;
        q.prepare("SELECT email FROM user_emails WHERE user_id = :id");
        q.bindValue(":id", curr_user_id);

        if (!q.exec())
            return emails;

        while (q.next())
            emails << q.value(0).toString();

        return emails;
    }


    Q_INVOKABLE void printTokens(){
        qDebug() << "email: " << m_userEmail << "\nname: " << m_userName << "\naccess: " << m_accestoken << "\nrefresh: " << m_refreshtoken;
    }

    Q_INVOKABLE QStringList listfiles(const QString &accessToken,
                                      const QString &pageToken = "")
    {
        QStringList result;

        QUrl url("https://www.googleapis.com/drive/v3/files");
        QUrlQuery q;
        q.addQueryItem("pageSize", "12");
        q.addQueryItem("fields", "nextPageToken,files(name)");

        if (!pageToken.isEmpty())
            q.addQueryItem("pageToken", pageToken);

        url.setQuery(q);

        QNetworkRequest req(url);
        req.setRawHeader("Authorization",
                         QString("Bearer %1").arg(accessToken).toUtf8());

        QNetworkReply *reply = m_manager.get(req);

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QByteArray data = reply->readAll();
        reply->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        m_nextPageToken = obj["nextPageToken"].toString();

        QJsonArray files = obj["files"].toArray();
        for (const QJsonValue &v : files)
            result << v.toObject()["name"].toString();

        return result;
    }

    Q_INVOKABLE QString getNextPageToken() const {
        return m_nextPageToken;
    }

    QString read_secret(){
        std::ifstream file("..\\android\\src\\com\\example\\untitled\\secret.txt");
        std::string str;
        while (std::getline(file, str))
        {
            return QString::fromStdString(str);
        }
        return "";
    }

    Q_INVOKABLE QString getAccessById(int index)
    {
        if (curr_user_id < 0 || index < 0)
            return "";

        QSqlQuery q;
        q.prepare(R"(
        SELECT access_token, refresh_token
        FROM user_emails
        WHERE user_id = :uid
        ORDER BY id
        LIMIT 1 OFFSET :idx
    )");
        q.bindValue(":uid", curr_user_id);
        q.bindValue(":idx", index);

        if (!q.exec() || !q.next())
            return "";

        QString accessToken = q.value(0).toString();
        QString refreshToken = q.value(1).toString();

        QNetworkRequest testReq(QUrl("https://www.googleapis.com/oauth2/v1/tokeninfo?access_token=" + accessToken));
        QNetworkReply *testReply = m_manager.get(testReq);

        QEventLoop loop;
        connect(testReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (testReply->error() == QNetworkReply::NoError) {
            testReply->deleteLater();
            return accessToken;
        }
        testReply->deleteLater();




        QNetworkRequest req(QUrl("https://oauth2.googleapis.com/token"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QUrlQuery body;
        body.addQueryItem("client_id",     "672191165584-lrc87gr0ip4ipra19s7k67tq8traagnm.apps.googleusercontent.com");
        body.addQueryItem("client_secret", read_secret());
        body.addQueryItem("refresh_token", refreshToken);
        body.addQueryItem("grant_type",    "refresh_token");

        QNetworkReply *reply = m_manager.post(req, body.toString(QUrl::FullyEncoded).toUtf8());

        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QByteArray resp = reply->readAll();
        reply->deleteLater();

        QJsonObject obj = QJsonDocument::fromJson(resp).object();
        QString newAccessToken = obj["access_token"].toString();

        if (newAccessToken.isEmpty())
            return "";




        QSqlQuery update;
        update.prepare(R"(
        UPDATE user_emails
        SET access_token = :at
        WHERE user_id = :uid AND refresh_token = :rt
    )");
        update.bindValue(":at", newAccessToken);
        update.bindValue(":uid", curr_user_id);
        update.bindValue(":rt", refreshToken);
        update.exec();

        return newAccessToken;
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
            uploadFileToDrive(p, getAccessById(upload_curr_id));
    }

    Q_INVOKABLE void uploadFileToDrive(const QString &uri, QString accessToken) {
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
        req.setRawHeader("Authorization", QString("Bearer %1").arg(accessToken).toUtf8());
        req.setRawHeader("Content-Type", QString("multipart/related; boundary=%1").arg(boundary).toUtf8());
        req.setHeader(QNetworkRequest::ContentLengthHeader, body.size());

        QNetworkReply *reply = m_manager.post(req, body);

        connect(reply, &QNetworkReply::finished, this, [this, reply, fileName]() {
            QByteArray resp = reply->readAll();
            qDebug() << resp;
            if (reply->error() == QNetworkReply::NoError) {
                emit fileUploadSuccess(fileName);
            }
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
