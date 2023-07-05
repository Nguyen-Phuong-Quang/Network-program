import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

Window {
    property int appWidth: 1080
    visible: true
    width: appWidth
    height: 800
    title: qsTr("Chat app")

    onWidthChanged: {
        appWidth = width
    }

    LoadingComponent {
        id: loadingId
        anchors.centerIn: parent
        visible: false
    }

    Rectangle {
        id: mainViewId
        anchors.fill: parent
        visible: false
        Rectangle {
            id: listRectId
            anchors {
                top: parent.top
            }
            width: appWidth * 0.25
            height: parent.height
            color: "#cccccc"

            ScrollView {
                anchors.fill: parent
                ListView {
                    id: userListId
                    anchors.top: parent.top
                    width: parent.width
                    height: contentHeight

                    model: client.getUserListVariant()
                    delegate: Component {
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: client && modelData.id !== client.get_current_user_id()
                            width: parent.width - 10
                            height: 52
                            color: "transparent"
                            Rectangle {
                                id: userViewId
                                width: parent.width
                                height: 44
                                anchors.centerIn: parent
                                radius: 10
                                Text {
                                    leftPadding: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.name
                                    font.pointSize: 12
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        client.switchChat(0, modelData.id)
                                    }
                                }
                            }

                            Rectangle {
                                anchors.top: userViewId.bottom
                                height: 10
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: chatViewRectId
            anchors {
                left: listRectId.right
                top: parent.top
            }
            width: appWidth * 0.75
            height: parent.height
            color: "#333333"
            ScrollView {
                id: messageScrollView
                anchors.fill: parent
                anchors.topMargin: 20
                anchors.bottomMargin: 60

                ListView {
                    id: chatListId
                    anchors.top: parent.top
                    width: parent.width
                    clip: true // Clip the content within the ListView
                    model: ListModel {}

                    onModelChanged: {
                        chatListId.contentY = chatListId.contentHeight - chatListId.height
                    }

                    delegate: Component {
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width - 40
                            height: 40
                            color: "transparent"
                            Text {
                                id: messageContentId
                                text: modelData.content
                                color: "white"
                                anchors.right: client && modelData.sender_id === client.get_current_user_id() ? parent.right : undefined
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: sendMessageRectId
                width: parent.width
                height: 60
                anchors.bottom: parent.bottom
                color: "#333333"
                Rectangle {
                    width: parent.width - 40
                    height: parent.height - 20
                    anchors.centerIn: parent
                    color: "transparent"

                    Rectangle {
                        width: parent.width - 70
                        height: parent.height
                        anchors.left: parent.left
                        radius: 50
                        TextInput {
                            id: chatTextInputId
                            leftPadding: 20
                            rightPadding: 20
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height - 20
                            font.pointSize: 12
                        }
                    }

                    Button {
                        width: 60
                        height: parent.height
                        anchors.right: parent.right
                        text: "Send"
                        onClicked: {
                            client.sendMessage(chatTextInputId.text);
                            chatTextInputId.text = "";
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: signInViewId
        anchors.fill: parent
        visible: true
        SignIn{
            id: signInId
        }
    }

    Connections {
        target: client

        onSignInResponse: {
            loadingId.visible = false
            if(statusCode === 200) {
                mainViewId.visible = true;
                signInViewId.visible = false;
            } else if(statusCode === 404) {
                signInId.error = "No user found!"
                console.log("No user found!");
            }else if(statusCode === 401) {
                signInId.error = "Wrong creadential!"
            }
        }

        onRender: {
            userListId.model = client.getUserListVariant()
        }

        onRenderChat: {
            chatListId.model = client.getChatVariant()
            chatListId.contentY = chatListId.contentHeight - chatListId.height
        }
    }

}
