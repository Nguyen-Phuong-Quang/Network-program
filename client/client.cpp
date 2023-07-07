#include "client.h"
#include <QDebug>
#include <QDataStream>

Client::Client(QObject *parent) : QObject(parent)
{
    // Create the socket
    socket = new QTcpSocket(this);

    // Move socket to the receive thread
    socket->moveToThread(&receiveThread);

    // Connect the readyRead signal to the readData slot
    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);

    // Start the receive thread
    receiveThread.start();

    // Connect to the TCP server upon initialization
    socket->connectToHost("127.0.0.1", 12345); // Connect to server IP and port
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

    receiveThread.quit();
    receiveThread.wait();
    socket->disconnectFromHost();
    socket->deleteLater(); // Cleanup the socket
}

void Client::readData()
{
    while (socket->bytesAvailable() > 0)
    {

        //Check type of response
        int type;
        QDataStream in(socket);
        in >> type;

        switch(type) {
        // Connect to server successfully
        case 200: {
            emit successConnection();
            qDebug() << "Database!";
            break;
        }
        case 400: {
            qDebug() << "Database fail!";
            break;
        }
        // Sign in
        case 1: {
            int code;
            in >> code;

            // If success
            if(code == 200) {
                in >> currentUserId;
                in >> name;
//                qDebug() << currentUserId << " " << name << "Sign in";

            }
            emit signInResponse(code);
            break;
        }
            // Users online
        case 2: {
            userListVariant.clear();
            QByteArray dataArray = socket->readAll();
            QDataStream stream(dataArray);

            // Deserialize the array size
            quint32 arraySize;
            stream >> arraySize;

            // Deserialize each struct in the array
            for (quint32 i = 0; i < arraySize; ++i) {
                User user;
                stream >> user.id;
                stream >> user.name;
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
            int code;
            in >> code;
            if(code == 200) {
                // Receive new chat data
                chatVariant.clear();
                QByteArray dataArray = socket->readAll();
                QDataStream stream(dataArray);

                // Deserialize the array size
                quint32 arraySize;
                stream >> arraySize;

                // Deserialize each struct in the array
                for (quint32 i = 0; i < arraySize; ++i) {
                    Message message;
                    stream >> message.sender_id;
                    stream >> message.name;
                    stream >> message.content;
                    QVariantMap messageMap;
                    messageMap["sender_id"] = message.sender_id;
                    messageMap["name"] = message.name;
                    messageMap["content"] = message.content;
                    chatVariant.append(messageMap);
                }
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
        }
        }

        emit render();
    }
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
    QDataStream out(socket);

    out << 1;
    out << username;
    out << password;
    socket->flush();

}

void Client::switchChat(int type ,int target_id, QString chatName) {
    typeSelected = type;

    if(type == 1) {
        nameSelected = chatName + " (ID: " + QString::number(target_id) + ")";
    } else {
        nameSelected = chatName;
    }

    // Send data to the server
    QDataStream out(socket);

    out << 2;
    out << type;
    out << target_id;
    socket->flush();
}

void Client::sendMessage(QString message) {

    QVariantMap messageMap;
    messageMap["sender_id"] = currentUserId;
    messageMap["name"] = name;
    messageMap["content"] = message;
    chatVariant.append(messageMap);

    emit renderChat();

    // Send data to the server
    QDataStream out(socket);
    out << 3;
    out << name;
    out << message;
    socket->flush();
}

void Client::createGroup(QString groupName) {
    QDataStream out(socket);
    out << 4;
    out << groupName;
    socket->flush();
}

void Client::requestJoinGroup(int groupId) {
    QDataStream out(socket);
    out << 5;
    out << groupId;
    socket->flush();
}
