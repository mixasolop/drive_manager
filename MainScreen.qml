import QtQuick
import QtQuick.Controls 2.15

Page {
    id: root
    anchors.fill: parent

    property var emails: backend.getUserEmails()
    property int currentIndex: 0

    property var files: []
    property string pageToken: ""

    Rectangle {
        anchors.fill: parent
        color: "#181c2c"
    }

    Column {
        anchors.centerIn: parent
        spacing: 30

        Text {
            text: "Your Emails"
            font.pixelSize: 32
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }

        Row {
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "<"
                enabled: currentIndex > 0
                onClicked: {
                    currentIndex--
                    files = []
                    pageToken = ""
                }
            }

            Rectangle {
                width: 320
                height: 480
                radius: 16
                color: "#262f51"

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        anchors.top: parent
                        text: emails.length > 0 ? emails[currentIndex] : "No emails"
                        color: "white"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }

                    Row{
                        spacing: 12
                        anchors.horizontalCenter: parent.horizontalCenter
                        Button {
                            text: "List files"
                            onClicked: {
                                // let token = backend.getAccessTokenByEmailIndex(currentIndex)
                                files = backend.listfiles(backend.getAccessById(currentIndex), "")
                                pageToken = backend.getNextPageToken()
                            }
                        }
                        Button {
                            text: "Pick files"
                            onClicked: {
                                backend.set_upload_accesstoken(currentIndex)
                                backend.pickFiles()
                            }
                        }
                    }
                    ListView {
                        height: 260
                        width: parent.width
                        model: files

                        delegate: Text {
                            text: modelData
                            color: "white"
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }
                    }

                    Row {
                        spacing: 10
                        anchors.horizontalCenter: parent.horizontalCenter

                        Button {
                            text: ">"
                            enabled: pageToken !== ""
                            onClicked: {
                                let token = backend.getAccessById(currentIndex)
                                files = backend.listfiles(token, pageToken)
                                pageToken = backend.getNextPageToken()
                            }
                        }
                    }
                }
            }

            Button {
                text: ">"
                enabled: currentIndex < emails.length - 1
                onClicked: currentIndex++
            }
        }

        Text {
            text: emails.length > 0
                  ? (currentIndex + 1) + " / " + emails.length
                  : ""
            color: "#aaaaaa"
            font.pixelSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Button {
            text: "Add email"
            width: 200
            onClicked: {
                backend.startAuth()
                stack.pop()
                stack.push("MainScreen.qml")
            }
        }
    }
}
