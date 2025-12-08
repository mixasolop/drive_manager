#include "backend.h"
#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>
#include <QMetaObject>
#include <QDebug>


Backend* backend = nullptr;

extern "C" JNIEXPORT void JNICALL
Java_com_example_untitled_QtBridge_onFilesSelected(JNIEnv *env,
                                                   jclass,
                                                   jobjectArray array)
{
    if (!backend) {
        qWarning() << "Backend instance is null!";
        return;
    }

    int count = env->GetArrayLength(array);
    QStringList paths;

    for (int i = 0; i < count; i++) {
        jstring jpath = (jstring)env->GetObjectArrayElement(array, i);
        const char* chars = env->GetStringUTFChars(jpath, nullptr);

        paths.append(QString::fromUtf8(chars));

        env->ReleaseStringUTFChars(jpath, chars);
        env->DeleteLocalRef(jpath);
    }

    QMetaObject::invokeMethod(
        backend,
        "onFilesSelected",
        Qt::QueuedConnection,
        Q_ARG(QStringList, paths)
        );
}
