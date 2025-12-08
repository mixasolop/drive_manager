import QtQuick
import QtQuick.Controls 2.15

ApplicationWindow  {
    width: 360
    height: 640
    visible: true
    title: qsTr("Timer")
    StackView{
        id: stack
        anchors.fill: parent
        pushEnter: null
        pushExit: null
        popEnter: null
        popExit: null
        replaceEnter: Transition {
                PropertyAnimation {
                    property: "y"
                    from: stack.height
                    to: 0
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }
        replaceExit: Transition {
                PropertyAnimation {
                    property: "y"
                    from: 0
                    to: -stack.height
                    duration: 300
                    easing.type: Easing.InCubic
                }
            }

        Component.onCompleted: {
                stack.push("Logger.qml")
        }
    }

    Connections{
        target: cppBackend;
        onLoginSuccess:{
            stack.push("MainScreen.qml")
        }
    }
}
