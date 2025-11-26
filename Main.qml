import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 400
    height: 400
    title: "Drive Manager"

    Button {
        text: "Login & request tokens"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 100
        onClicked: backend.startAuth()
    }

    Button {
        text: "Show tokens"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 300
        onClicked: {
            console.log("Access:", backend.getAccessToken())
            console.log("Refresh:", backend.getRefreshToken())
            backend.printTokens()
        }
    }

    Button {
        text: "list files"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 400
        onClicked: backend.listfiles()
    }
    Button {
        text: "print token"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 600
        onClicked: backend.printAccess()
    }
}
