#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "backend.h"

extern Backend* backend;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Backend backendInstance;
    qDebug() << "Backend instance pointer:" << &backendInstance;

    backend = &backendInstance;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("appdata.db");   // file will be created if not exists

    if (!db.open()) {
        qWarning() << "Failed to open DB:" << db.lastError().text();
    }
    else{qDebug() << "Connected to db";}

    QSqlQuery query;

    query.exec(R"(
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        salt TEXT NOT NULL,
        password_hash TEXT NOT NULL
    );
    )");

    query.exec(R"(
    CREATE TABLE IF NOT EXISTS user_emails (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        email TEXT NOT NULL,
        access_token TEXT,
        refresh_token TEXT,
        FOREIGN KEY(user_id) REFERENCES users(id)
    );
    )");



    engine.rootContext()->setContextProperty("backend", &backendInstance);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
        );

    engine.loadFromModule("untitled", "Main");


    return app.exec();
}
