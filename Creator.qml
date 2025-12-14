import QtQuick
import QtQuick.Controls 2.15

Page {
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#181c2c"

        Text {
            text: "Sing In"
            font.pixelSize: 42
            font.bold: true
            color: "#e4e4e4"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: parent.height * 0.15
        }

        Rectangle {
            width: parent.width * 0.9
            height: parent.height * 0.42
            color: "#262f51"
            radius: 40
            anchors.centerIn: parent

            Column {
                spacing: 16
                anchors.centerIn: parent
                width: parent.width * 0.85

                Text { text: "Username"; color: "white"; font.pixelSize: 14 }

                TextInput {
                    id: texts
                    width: parent.width
                    height: 36
                    color: "white"
                }

                Text {
                    text: "Password"
                    color: "white"
                    font.pixelSize: 14
                }

                TextInput {
                    id: texts1
                    width: parent.width
                    height: 36
                    color: "white"
                    echoMode: TextInput.Password
                }

                Button {
                    width: parent.width
                    height: 48
                    background: Rectangle { color: "transparent" }

                    Text {
                        anchors.centerIn: parent
                        text: "Sing In"
                        color: "#e4e4e4"
                        font.pixelSize: 28
                        font.bold: true
                    }
                    onClicked: {
                        backend.addUser(texts.text, texts1.text)
                        stack.pop()
                    }
                }
            }
        }
    }
}
