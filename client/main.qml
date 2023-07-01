import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

Window {
    visible: true
    width: 860
    height: 640
    title: qsTr("Chat app")

    signal changeToUser(int it)

    function demoMessage1() {
        return [
                    {
                        sender_id: 1,
                        message: "Hello"
                    },
                    {
                        sender_id: 2,
                        message: "Hi"
                    },
                    {
                        sender_id: 1,
                        message: "My nams is Nguyen Phuong Quang"
                    },
                    {
                        sender_id: 2,
                        message: "Hi Quang, i'm Giang"
                    },
                    {
                        sender_id: 1,
                        message: "Nice to meet you"
                    },
                    {
                        sender_id: 2,
                        message: "How do you feel right now?"
                    },
                    {
                        sender_id: 1,
                        message: "I'm good"
                    }
                ]
    }

    function demoMessage2() {
        return [
                    {
                        sender_id: 1,
                        message: "hiuebifbu"
                    },
                    {
                        sender_id: 2,
                        message: "Hi"
                    },
                    {
                        sender_id: 1,
                        message: "My nams is Nguyen Phuong Quang"
                    },
                    {
                        sender_id: 2,
                        message: "Hi Quang, i'm Giang"
                    },
                    {
                        sender_id: 1,
                        message: "Nice to meet you"
                    },
                    {
                        sender_id: 2,
                        message: "How do you feel right now?"
                    },
                    {
                        sender_id: 1,
                        message: "I'm good"
                    }
                ]
    }


    function handleSelectUser(modelData) {
        console.log(modelData.id + " " + modelData.name)

        if(modelData.id === 1) {
            chatListId.model = demoMessage1()
        }

        if(modelData.id === 2) {
            chatListId.model = demoMessage2()
        }
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
            width: 200
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
                                        handleSelectUser(modelData)
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
            width: 660
            height: parent.height
            color: "#333333"
            ScrollView {
                id: messageScrollView
                anchors.fill: parent
                anchors.topMargin: 20
                anchors.bottomMargin: 60
                ListView {
                    id: chatListId
                    anchors.top: parent.to
                    width: parent.width
                    height: contentHeight
                    model: []
                    delegate: Component {
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width - 40
                            height: 40
                            color: "transparent"
                            Text {
                                id: messageContentId
                                text: modelData.message
                                color: "white"
                                anchors.right: modelData.sender_id === client.get_current_user_id() && parent.right
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
                color: "transparent"
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

        }
    }

}
