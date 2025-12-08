cmake_minimum_required(VERSION 3.16)

project(untitled VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appuntitled
    main.cpp
    backend.h
)

qt_add_qml_module(appuntitled
    URI untitled
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES backend_jni.cpp
        QML_FILES Logger.qml
)

set_property(TARGET appuntitled APPEND PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/android
)
if(ANDROID)
    set(OPENSSL_ANDROID_LIBS
        ${CMAKE_SOURCE_DIR}/android/openssl/${ANDROID_ABI}/libssl_3.so
        ${CMAKE_SOURCE_DIR}/android/openssl/${ANDROID_ABI}/libcrypto_3.so
    )

    set_property(TARGET appuntitled APPEND PROPERTY
        QT_ANDROID_EXTRA_LIBS
            ${OPENSSL_ANDROID_LIBS}
    )
endif()

target_link_libraries(appuntitled PRIVATE Qt6::Quick Qt6::Core Qt6::Network)
