import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3


Rectangle {
    anchors.centerIn: parent
    width: 400
    height: 250
    border {
        width: 2
        color: "#cccccc"
    }
    radius: 20

    function handleOnClick() {
        mainViewId.visible = true
        signInViewId.visible = false

        console.log(username.text + " " + password.text)
    }

    Text {
        id: signInTextId
        topPadding: 10
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Sign in"
        font.pointSize: 16
        font.bold: true
    }

    Text {
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        id: usernameTextId
        anchors.top: signInTextId.bottom
        text: "Username"

    }

    Rectangle {
        id: usernameInputId
        anchors.top: usernameTextId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#E1D8D6"
        height: 40
        width: parent.width - 40
        radius: 16
        TextInput {
            id: username
            leftPadding: 20
            width: parent.width
            anchors.centerIn: parent
        }
    }

    Text {
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        id: passwordTextId
        anchors.top: usernameInputId.bottom
        text: "Password"

    }

    Rectangle {
        id: passwordInputId
        anchors.top: passwordTextId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#E1D8D6"
        height: 40
        width: parent.width - 40
        radius: 16
        TextInput {
            id: password
            leftPadding: 20
            width: parent.width
            anchors.centerIn: parent
        }
    }

    Rectangle {
        id: signInGapId
        anchors.top: passwordInputId.bottom
        height: 16
    }

    Rectangle {
        anchors.top: signInGapId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#3AE5D3"
        width: 100
        height: 30
        radius: 4
        Text {
            anchors.centerIn: parent
            text: qsTr("Sign in")
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.OpenHandCursor
            onClicked: {
                handleOnClick()
            }
        }
    }
}