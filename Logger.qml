import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: view
    width: 400
    height: 700

    Column {
        anchors.centerIn: parent
        spacing: 20

        Button {
            text: "Login"
            width: 200
            onClicked: backend.startAuth()
        }
        Button {
            text: "Show tokens"
            onClicked: {
                console.log("Access:", backend.getAccessToken())
                console.log("Refresh:", backend.getRefreshToken())
                backend.printTokens()
            }
        }

        Button {
            text: "List Files"
            width: 200
            onClicked: backend.listfiles()
        }

        Button {
            text: "Pick Files"
            width: 200
            onClicked: backend.pickFiles()
        }

        Text {
            text: "Google Drive Uploader"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
