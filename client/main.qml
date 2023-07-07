import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Window {
    property int appWidth: 1080
    visible: true
    width: appWidth
    height: 800
    title: qsTr("Chat app")

    function demoUsers () {
        return [
                    {
                        id: 1,
                        name: "Test group"
                    }
                ]
    }

    function demoMessage () {
        return [
                    {
                        sender_id: 1,
                        content: "Quang content",
                        name: "Quang"
                    },
                    {
                        sender_id: 2,
                        content: "Hai content",
                        name: "Hai"
                    }
                ]
    }

    function openFindGroupDialog() {
        joinGroupDialogId.visible = true
        mainViewId.opacity = 0.8
    }

    function hideFindGroupDialog() {
        joinGroupDialogId.visible = false
        mainViewId.opacity = 1
        joinGroupInputId.text = ""
    }

    function openAddGroupDialog() {
        addGroupDialogId.visible = true
        mainViewId.opacity = 0.8
    }

    function hideAddGroupDialog() {
        addGroupDialogId.visible = false
        mainViewId.opacity = 1
        addGroupInputId.text = ""
    }

    onWidthChanged: {
        appWidth = width
    }

    // Dialog add group with id
    Rectangle {
        id: addGroupDialogId
        visible: false
        z: 10
        anchors.fill: parent
        color: "transparent"
        MouseArea {
            anchors.fill: parent
            preventStealing: true // Prevent interaction from propagating to underlying items
        }

        Rectangle {
            z: 999
            color: "#333333"
            anchors.centerIn: parent
            width: 300
            height: 200
            radius: 16
            border.width: 1
            border.color: "white"

            Text {
                id: addGroupTextId
                padding: 10
                text: "Create group by name"
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                font.pointSize: 14
            }

            Rectangle {
                anchors.top: addGroupTextId.bottom
                width: parent.width
                height: 1
                color: "#cccccc"

            }

            Rectangle {
                id: addGroupIdInputRectId
                anchors.top: addGroupTextId.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 28
                width: parent.width - 100
                height: 40
                radius: 16

                TextInput {
                    id: addGroupInputId
                    leftPadding: 20
                    rightPadding: 20
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height - 12
                    font.pointSize: 16
                }
            }

            Text {
                id: errorAddGroupTextId
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: addGroupIdInputRectId.bottom
                topPadding: 10
                text: ""
                color: "red"
                font.pointSize: 12
            }

            Button {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    bottomMargin: 10
                    rightMargin: 30
                }
                text: "Submit"
                onClicked: {
                    if(!addGroupInputId.text) {
                        errorAddGroupTextId.text = "Please enter group name!"
                    } else {
                        client.createGroup(addGroupInputId.text);
                    }
                }
            }

            Button {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    bottomMargin: 10
                    leftMargin: 30
                }
                text: "Cancel"
                onClicked: {
                    hideAddGroupDialog()
                }
            }
        }
    }

    // Dialog join group with id
    Rectangle {
        id: joinGroupDialogId
        visible: false
        z: 10
        anchors.fill: parent
        color: "transparent"
        MouseArea {
            anchors.fill: parent
            preventStealing: true // Prevent interaction from propagating to underlying items
        }

        Rectangle {
            z: 999
            color: "#333333"
            anchors.centerIn: parent
            width: 300
            height: 200
            radius: 16
            border.width: 1
            border.color: "white"

            Text {
                id: joinGroupTextId
                padding: 10
                text: "Join group by ID"
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                font.pointSize: 14
            }

            Rectangle {
                anchors.top: joinGroupTextId.bottom
                width: parent.width
                height: 1
                color: "#cccccc"

            }

            Rectangle {
                id: groupIdInputRectId
                anchors.top: joinGroupTextId.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 28
                width: parent.width - 100
                height: 40
                radius: 16

                TextInput {
                    id: joinGroupInputId
                    leftPadding: 20
                    rightPadding: 20
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height - 12
                    font.pointSize: 16
                }
            }

            Text {
                id: errorJoinGroupTextId
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: groupIdInputRectId.bottom
                topPadding: 10
                text: ""
                color: "red"
                font.pointSize: 12
            }

            Button {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    bottomMargin: 10
                    rightMargin: 30
                }
                text: "Request"
                onClicked: {
                    if(!joinGroupInputId.text) {
                        errorJoinGroupTextId.text = "Please enter group ID!"
                    } else if(!parseInt(joinGroupInputId.text)) {
                        errorJoinGroupTextId.text = "Group ID just contains digit!"
                    } else {
                        client.requestJoinGroup(parseInt(joinGroupInputId.text));
                    }
                }
            }

            Button {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    bottomMargin: 10
                    leftMargin: 30
                }
                text: "Cancel"
                onClicked: {
                    hideFindGroupDialog();
                }
            }
        }
    }

    LoadingComponent {
        id: loadingId
        anchors.centerIn: parent
        visible: true
    }

    Rectangle {
        id: mainViewId
        anchors.fill: parent
        visible: false
        opacity: 1
        Rectangle {
            id: listRectId
            anchors {
                top: parent.top
            }
            width: appWidth * 0.25
            height: parent.height
            color: "#333333"

            Rectangle {
                width: 1
                height: parent.height
                color: "#cccccc"
                anchors.right: parent.right
            }

            Text {
                id: onlineTextId
                text: "Online"
                anchors.top: parent.top
                padding: 10
                color: "white"
            }

            // List of online user
            ScrollView {
                id: onlineUserScrollId
                anchors.top: onlineTextId.bottom
                width: parent.width
                height: parent.height * 1/2
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff  // Hide the vertical scroll bar
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff  // Hide the horizontal scroll bar
                clip: true
                ListView {
                    id: userListId
                    anchors.top: parent.top
                    width: parent.width
                    height: contentHeight
                    clip: true
                    model: []
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
                                        client.switchChat(0, modelData.id, modelData.name)
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

            Rectangle {
                id: gapUserAndGroupId
                anchors.top: onlineUserScrollId.bottom
                height: 5
                width: parent.width
                color: "transparent"
                Rectangle {
                    width: parent.width
                    height: 1
                    anchors.centerIn: parent
                    color: "white"
                }
            }

            Rectangle {
                id: groupRectId
                anchors.top: gapUserAndGroupId.bottom
                height: 40
                width: parent.width
                color: "transparent"

                Text {
                    padding: 10
                    text: "Group"
                    color: "white"
                }

                Rectangle {
                    id: marginLeftButtonId
                    anchors.right: parent.right
                    width: 8
                }

                Button {
                    id: createGroupButtonId
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: marginLeftButtonId.left
                    height: parent.height - 10
                    width: 40
                    text: "+"
                    onClicked: {
                        openAddGroupDialog()
                    }
                }
                Rectangle {
                    id: gapButtonGroupId
                    anchors.right: createGroupButtonId.left
                    width: 10
                }

                Button {
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height - 10
                    width: 40
                    anchors.right: gapButtonGroupId.left
                    icon.source: "./find.svg"
                    onClicked: {
                        openFindGroupDialog();
                    }
                }
            }

            // List of group
            ScrollView {
                id: groupScrollId
                anchors.top: groupRectId.bottom
                width: parent.width
                height: parent.height * 4/10
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff  // Hide the vertical scroll bar
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff  // Hide the horizontal scroll bar
                clip: true
                ListView {
                    id: groupListId
                    anchors.top: parent.top
                    width: parent.width
                    height: contentHeight
                    clip: true
                    model: []
                    delegate: Component {
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width - 10
                            height: 52
                            color: "transparent"
                            Rectangle {
                                id: groupViewId
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
                                        client.switchChat(1, modelData.id, modelData.name)
                                    }
                                }
                            }

                            Rectangle {
                                anchors.top: groupViewId.bottom
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

            Rectangle {
                id: infoChatId
                anchors.top: parent.top
                width: parent.width
                height: 40
                color: "transparent"

                Rectangle {
                    height: 1
                    width: parent.width
                    anchors.bottom: parent.bottom
                    color: "#cccccc"
                }

                Text {
                    id: chatNameTextId
                    leftPadding: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: ""
                    font.pointSize: 16
                    color: "white"
                }

                Rectangle {
                    id: marginLeftGroupBtnId
                    anchors.right: parent.right
                    width: 5
                }

                Button {
                    id: leftGroupBtnId
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height - 10
                    width: 40
                    anchors.right: marginLeftGroupBtnId.left
                    icon.source: "./leftGroup.png"
                }
            }

            ScrollView {
                id: messageScrollView
                anchors.fill: parent
                anchors.topMargin: 40
                anchors.bottomMargin: 60
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff  // Hide the horizontal scroll bar
                clip: true

                ListView {
                    id: chatListId
                    anchors.top: parent.top
                    width: parent.width
                    topMargin: 10
                    clip: true // Clip the content within the ListView
                    model: ListModel {}

                    //                    model: demoMessage()

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
                                id: chatFriendNameId
                                visible: client && modelData.sender_id !== client.get_current_user_id()
                                topPadding: 6
                                bottomPadding: 6
                                rightPadding: 2
                                text: modelData.name + ": "
                                color: "white"
                            }

                            Rectangle {
                                anchors.left: client && modelData.sender_id !== client.get_current_user_id() ? chatFriendNameId.right : undefined
                                anchors.right: client && modelData.sender_id === client.get_current_user_id() ? parent.right : undefined
                                width: messageContentId.implicitWidth
                                height: messageContentId.implicitHeight
                                color: "white"
                                radius: 10
                                Text {
                                    padding: 6
                                    id: messageContentId
                                    text: modelData.content
                                    color: "black"
                                }
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
        visible: false
        SignIn{
            id: signInId
        }
    }

    Connections {
        target: client

        onSuccessConnection: {
            signInViewId.visible = true;
            loadingId.visible = false;
        }

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
            chatNameTextId.text = client.getNameSelected();
            leftGroupBtnId.visible = client.getTypeSelected() === 1;
            userListId.model = client.getUserListVariant()
            groupListId.model = client.getGroupListVariant()
        }

        onRenderChat: {
            chatListId.model = client.getChatVariant()
            chatListId.contentY = chatListId.contentHeight - chatListId.height
        }

        onCreateGroupResponse: {
            if(code === 200) {
                hideAddGroupDialog();
            } else if (code === 409) {
                errorAddGroupTextId.text = "Group name is already used!"
            }
        }
    }

}
