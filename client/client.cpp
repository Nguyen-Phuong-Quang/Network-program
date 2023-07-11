#include "client.h"
#include <QDebug>
#include <QDataStream>

Client::Client(QObject *parent) : QObject(parent)
{
    // Create the socket
    socket = new QTcpSocket(this);

    // Connect the readyRead signal to the readData slot
    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);


    // Connect to the TCP server upon initialization
    socket->connectToHost("127.0.0.1", 5050); // Connect to server IP and port
    if (socket->waitForConnected())
    {
        qDebug() << "Connected to server";
    }
    else
    {
        qDebug() << "Failed to connect to server";
    }
}

Client::~Client()
{
    QDataStream stream(socket);

    stream << 0;
    socket->flush();

    socket->disconnectFromHost();
    socket->deleteLater(); // Cleanup the socket
}

void Client::readData()
{   mutex.lock();
    QByteArray data = socket->readAll();

    QDataStream in(&data, QIODevice::ReadOnly);
    //Check type of response
    int type;
    in >> type;

    qDebug() << "Type" << type;
    switch(type) {
    // Sign in
    case 1: {
        int code;
        in >> code;

        // If success
        if(code == 200) {
            in >> currentUserId;
            in >> name;
        }

        emit signInResponse(code);
        break;
    }
        // Users online
    case 2: {
        userListVariant.clear();
        quint32 arraySize;
        in >> arraySize;

        // Deserialize each struct in the array
        for (quint32 i = 0; i < arraySize; ++i) {
            User user;
            in >> user.id;
            in >> user.name;
            if(user.id != currentUserId) {
                QVariantMap userMap;
                userMap["id"] = user.id;
                userMap["name"] = user.name;
                userListVariant.append(userMap);
            }
        }
        break;
    }
        // Switch message
    case 3: {
        // Receive new chat data
        chatVariant.clear();
        // Deserialize the array size
        quint32 arraySize;
        in >> arraySize;

        // Deserialize each struct in the array
        for (quint32 i = 0; i < arraySize; ++i) {
            Message message;
            in >> message.sender_id;
            in >> message.name;
            in >> message.content;
            QVariantMap messageMap;
            messageMap["sender_id"] = message.sender_id;
            messageMap["name"] = message.name;
            messageMap["content"] = message.content;
            chatVariant.append(messageMap);
        }
        emit renderChat();
        break;
    }
        // Send message
    case 4: {
        Message message;
        in >> message.sender_id;
        in >> message.name;
        in >> message.content;

        QVariantMap messageMap;
        messageMap["sender_id"] = message.sender_id;
        messageMap["name"] = message.name;
        messageMap["content"] = message.content;
        chatVariant.append(messageMap);
        emit renderChat();
        break;
    }
        // Groups
    case 5: {
        groupListVariant.clear();
        int size;
        in >> size;
        for(int i = 0; i < size; ++i) {
            int groupId;
            QString groupName;
            in >> groupId;
            in >> groupName;
            QVariantMap data;
            data["id"] = groupId;
            data["name"] = groupName;
            groupListVariant.append(data);
        }
        break;

    }
        // Create groups response
    case 6: {
        int code;
        in >> code;
        emit createGroupResponse(code);
        break;
    }
        // Join group response
    case 7: {
        int  code;
        in >> code;
        emit joinGroupResponse(code);
        break;
    }
    case 8: {
        requestListVariant.clear();
        int size;
        in >> size;

        for(int i = 0; i < size; ++i) {
            User newUser;
            in >> newUser.id;
            in >> newUser.name;
            QVariantMap data;
            data["id"] = newUser.id;
            data["name"] = newUser.name;
            requestListVariant.append(data);
        }
        qDebug() << "Have " << size << " requests";
        emit renderRequestList();
        break;
    }
        // Left group response
    case 9: {
        emit hideChatView();
        break;
    }
    case 10: {
        emit hideChatView();
        break;
    }
    case 11: {
        emit navigateToSignIn();
        break;
    }
    case 12: {
        int code;
        in >> code;
        emit errorSignUp(code);
        break;
    }
    }

    if(in.atEnd()) {
        qDebug() << "Clear data in socket" << Qt::endl;
    }

    emit render();
    mutex.unlock();

}

void Client::sendDataToServer(const QByteArray& data)
{
    mutex.lock();
    while (socket->state() != QAbstractSocket::ConnectedState) {
    }

    socket->write(data);
    socket->waitForBytesWritten();
    //    qDebug() << "Send data";
    mutex.unlock();
}

QString Client::getName() const
{
    return name;
}

QVariantList Client::getRequestListVariant() const
{
    return requestListVariant;
}
int Client::getTypeSelected() const
{
    return typeSelected;
}

QString Client::getNameSelected() const
{
    return nameSelected;
}

QVariantList Client::getGroupListVariant()
{
    return groupListVariant;
}

QVariantList Client::getChatVariant() const
{
    return chatVariant;
}

int Client::get_current_user_id() {
    return currentUserId;
}

QVariantList Client::getUserListVariant()
{
    return userListVariant;
}

void Client::signIn(QString username, QString password) {
    // Send data to the server
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 1 << username << password;

    sendDataToServer(data);
}

void Client::switchChat(int type ,int target_id, QString chatName) {
    typeSelected = type;
    targetId = target_id;
    if(type == 1) {
        nameSelected = chatName + " (ID: " + QString::number(target_id) + ")";
    } else {
        nameSelected = chatName;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 2 << type << target_id;

    sendDataToServer(data);
}

void Client::sendMessage(QString message) {

    QVariantMap messageMap;
    messageMap["sender_id"] = currentUserId;
    messageMap["name"] = name;
    messageMap["content"] = message;
    chatVariant.append(messageMap);

    emit renderChat();

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 3 << name << message;

    sendDataToServer(data);
}

void Client::createGroup(QString groupName) {

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 4 << groupName;

    sendDataToServer(data);
}

void Client::requestJoinGroup(int groupId) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 5 << groupId;

    sendDataToServer(data);
}

void Client::getPendingRequests() {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 6 << targetId;

    sendDataToServer(data);
}

void Client::acceptOrRejectUser(int type, int userId) {

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 7 << type << targetId << userId;

    sendDataToServer(data);
};

void Client::leftGroup() {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 8 << currentUserId << targetId;

    sendDataToServer(data);
}

void Client::signOut()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 9;

    sendDataToServer(data);

}

void Client::signUp(QString username, QString password, QString name)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 10 << username << password << name;
    sendDataToServer(data);
}

void Client::switchClientView(int type)
{
    if(type == 1) {
        emit navigateToSignIn();
    }

    if(type == 2) {
        emit navigateToSignUp();
    }
};

