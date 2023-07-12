import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

Rectangle {
    property string messageSignUp: "wqewq"
    property string colorSignUp: "green"
    anchors.centerIn: parent
    width: 400
    height: 380
    border {
        width: 2
        color: "#cccccc"
    }
    radius: 20

    function handleOnClick() {
        if(usernameSignUp.text && passwordSignUp.text && nameSignUp.text) {
            client.signUp(usernameSignUp.text, passwordSignUp.text, nameSignUp.text);
            usernameSignUp.text = ""
            passwordSignUp.text = ""
            nameSignUp.text = ""
        } else {
            signUpId.error = "Missing field!"
        }

    }

    Text {
        id: signUpTextId
        topPadding: 10
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Sign up"
        font.pointSize: 16
        font.bold: true
    }

    Text {
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        bottomPadding: 2
        id: usernameSignUpTextId
        anchors.top: signUpTextId.bottom
        text: "Username"

    }

    Rectangle {
        id: usernameSignUpInputId
        anchors.top: usernameSignUpTextId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#E1D8D6"
        height: 40
        width: parent.width - 40
        radius: 16
        TextInput {
            id: usernameSignUp
            leftPadding: 20
            width: parent.width
            anchors.centerIn: parent
        }
    }

    Text {
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        bottomPadding: 2
        id: passwordSignUpTextId
        anchors.top: usernameSignUpInputId.bottom
        text: "Password"

    }

    Rectangle {
        id: passwordSignUpInputId
        anchors.top: passwordSignUpTextId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#E1D8D6"
        height: 40
        width: parent.width - 40
        radius: 16
        TextInput {
            id: passwordSignUp
            leftPadding: 20
            width: parent.width
            anchors.centerIn: parent
            echoMode: TextInput.Password
        }
    }

    Text {
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        bottomPadding: 2
        id: nameSignUpTextId
        anchors.top: passwordSignUpInputId.bottom
        text: "Name"

    }

    Rectangle {
        id: nameSignUpInputId
        anchors.top: nameSignUpTextId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#E1D8D6"
        height: 40
        width: parent.width - 40
        radius: 16
        TextInput {
            id: nameSignUp
            leftPadding: 20
            width: parent.width
            anchors.centerIn: parent
        }
    }

    Rectangle {
        id: signUpGapId
        anchors.top: nameSignUpInputId.bottom
        height: 20
    }

    Rectangle {
        id: signUpButtonId
        anchors.top: signUpGapId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#3AE5D3"
        width: 100
        height: 30
        radius: 4
        Text {
            anchors.centerIn: parent
            text: qsTr("Sign up")
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

    Rectangle {
        anchors.top: signUpButtonId.bottom
        height: 16
        color: "transparent"
        width: parent.width
        Text {
            id: errorTextId
            topPadding: 5
            anchors.horizontalCenter: parent.horizontalCenter
            text: messageSignUp
            color: colorSignUp
        }
    }

    Text{
        padding: 10
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("You already have account? Sign in here!")
        color: "blue"
        font.underline: true
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                client.switchClientView(1);
            }
        }
    }
}
