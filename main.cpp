#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "backend.h"

// Эта переменная определена в backend_jni.cpp
extern Backend* backend;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // создаём единственный экземпляр Backend
    Backend backendInstance;
    qDebug() << "Backend instance pointer:" << &backendInstance;

    // инициализируем глобальный указатель для JNI
    backend = &backendInstance;

    // пробрасываем backend в QML
    engine.rootContext()->setContextProperty("backend", &backendInstance);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
        );

    engine.loadFromModule("untitled", "Main");

    backendInstance.printTokens();

    return app.exec();
}
